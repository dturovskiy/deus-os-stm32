import copy, hashlib, os, pathlib, sys, tempfile, unittest
sys.dont_write_bytecode = True  # Read-only test must not dirty the tracked source checkout.
from artifact_ledger import Refuse, validate_registry, report, root_map, SCHEMA

class RegistryTests(unittest.TestCase):
    def setUp(self):
        self.t = tempfile.TemporaryDirectory()
        self.addCleanup(self.t.cleanup)
        self.root=pathlib.Path(self.t.name)
        self.name='signed_firmware_v5.pkg'
        self.path=self.root/self.name
        self.path.write_bytes(b'valid synthetic firmware only')
        self.reg={'schema':SCHEMA,'entries':[{'id':'rev5_signed_pkg','kind':'SIGNED_APPLICATION_PACKAGE','custody':'HISTORICAL_REFERENCE_ONLY','sha256':hashlib.sha256(self.path.read_bytes()).hexdigest().upper(),'bytes':self.path.stat().st_size,'locators':[{'alias':'WINDOWS_DOWNLOADS','relative':self.name}]}]}
    def check_rejected(self,mut,code=None):
        changed=copy.deepcopy(self.reg)
        mut(changed)
        with self.assertRaises(Refuse) as err: validate_registry(changed)
        if code:self.assertEqual(str(err.exception),code)
    def test_valid_positive(self):
        self.assertEqual(validate_registry(self.reg),1)
        self.assertEqual(report(self.reg,{'WINDOWS_DOWNLOADS':self.root})['exact_matches'],1)
    def test_bad_sha(self):self.check_rejected(lambda r:r['entries'][0].update(sha256='f'),'ENTRY_DIGEST')
    def test_duplicate_id(self):self.check_rejected(lambda r:r['entries'].append(copy.deepcopy(r['entries'][0])),'DUPLICATE_ENTRY_ID')
    def test_path_escape(self):self.check_rejected(lambda r:r['entries'][0]['locators'][0].update(relative='../out.pkg'),'PATH_TRAVERSAL')
    def test_absolute_path(self):self.check_rejected(lambda r:r['entries'][0]['locators'][0].update(relative='C:/users/out.pkg'),'RELATIVE_PATH_FORBIDDEN_CHAR')
    def test_secret_extension(self):self.check_rejected(lambda r:r['entries'][0]['locators'][0].update(relative='device.key'),'SECRET_OR_UNSUPPORTED_FILE_TYPE')
    def test_bad_alias(self):self.check_rejected(lambda r:r['entries'][0]['locators'][0].update(alias='user-home'),'LOCATOR_ALIAS')
    def test_incomplete_identity(self):self.check_rejected(lambda r:r['entries'][0].update(bytes=None),'INCOMPLETE_IDENTITY')
    def test_duplicate_locator(self):self.check_rejected(lambda r:r['entries'][0]['locators'].append(copy.deepcopy(r['entries'][0]['locators'][0])),'DUPLICATE_LOCATOR')
    def test_root_unmapped_not_green(self):
        x=report(self.reg,{})
        self.assertEqual(x['results'][0]['status'],'NOT_VERIFIED')
        self.assertEqual(x['exact_matches'],0)
    def test_missing_file_not_green(self):
        self.path.unlink()
        self.assertEqual(report(self.reg,{'WINDOWS_DOWNLOADS':self.root})['results'][0]['status'],'NOT_VERIFIED')
    def test_size_drift(self):
        self.path.write_bytes(b'x')
        self.assertEqual(report(self.reg,{'WINDOWS_DOWNLOADS':self.root})['results'][0]['status'],'MISMATCH')
    def test_sha_drift(self):
        self.path.write_bytes(b'X'*self.reg['entries'][0]['bytes'])
        self.assertEqual(report(self.reg,{'WINDOWS_DOWNLOADS':self.root})['results'][0]['status'],'MISMATCH')
    def test_symlink_rejected(self):
        self.path.unlink()
        outside = self.root/'outside.pkg';outside.write_bytes(b'S'*9)
        self.path.symlink_to(outside)
        x=report(self.reg,{'WINDOWS_DOWNLOADS':self.root})
        self.assertEqual(x['results'][0]['locators'][0]['status'],'ACCESS_OR_POLICY_REJECTED')
    def test_verified_without_locator(self):self.check_rejected(lambda r:r['entries'][0].update(custody='VERIFIED_CUSTODY',locators=[]),'VERIFIED_WITHOUT_LOCATOR')
    def test_bound_root_not_absolute(self):
        with self.assertRaises(Refuse): root_map(['WINDOWS_DOWNLOADS=relative'])
    def test_drive_root_forbidden(self):
        with self.assertRaises(Refuse): root_map(['WINDOWS_DOWNLOADS='+str(pathlib.Path(self.root.anchor))])
    def test_symlink_root_forbidden(self):
        link = self.root.parent / ('deus_ledger_test_root_'+self.root.name)
        self.addCleanup(lambda: link.unlink(missing_ok=True))
        link.symlink_to(self.root, target_is_directory=True)
        with self.assertRaises(Refuse): root_map(['WINDOWS_DOWNLOADS='+str(link)])
    def test_secret_data_not_in_result(self):
        x=str(report(self.reg,{'WINDOWS_DOWNLOADS':self.root}))
        self.assertNotIn(str(self.root),x)
        self.assertNotIn('valid synthetic firmware only',x)
    def test_other_types_valid(self):
        self.reg['entries'][0]['kind']='UNSIGNED_APPLICATION_BIN'
        self.assertEqual(validate_registry(self.reg),1)
    def test_root_alias_collision(self):
        with self.assertRaises(Refuse):root_map(['WINDOWS_DOWNLOADS='+str(self.root)]*2)
    def test_unpinned_never_green(self):
        self.reg['entries'][0]['bytes']=None;self.reg['entries'][0]['sha256']=None
        self.assertEqual(report(self.reg,{'WINDOWS_DOWNLOADS':self.root})['exact_matches'],0)

if __name__=='__main__':unittest.main(verbosity=2)
