#!/usr/bin/env python3
"""No-network source regression: exact pinned wheel acquisition/receipt semantics."""
import base64
import csv
import hashlib
import io
import json
from pathlib import Path
from unittest import mock
import tempfile
import unittest
import zipfile

import download_a05_cp314_pypi_wheels_v1 as dl

def sha(x):return hashlib.sha256(x).hexdigest().upper()
PACKAGES=[
 ("cryptography","50.0.2","cryptography-50.0.2-cp311-abi3-win_amd64.whl",
  "cp311-abi3-win_amd64","cffi>=2.0.0"),
 ("cffi","2.1.1","cffi-2.1.1-cp314-cp314-win_amd64.whl",
  "cp314-cp314-win_amd64","pycparser"),
 ("pycparser","3.12","pycparser-3.12-py3-none-any.whl",
  "py3-none-any",None)
]

def fixture(name,version,tag,requires):
    prefix=name+"-"+version+".dist-info/"
    contents={
       name+"/fixture_nonexec_data.txt":b"x"*1400,
       prefix+"WHEEL":("Wheel-Version: 1.0\nGenerator: DEUS_SYNTHETIC_TEST\n"
                       "Root-Is-Purelib: false\nTag: "+tag+"\n").encode(),
       prefix+"METADATA":("Metadata-Version: 2.3\nName: "+name+
                          "\nVersion: "+version+"\n"+
                          ("Requires-Dist: "+requires+"\n" if requires else "")).encode()
    }
    rows=[]
    for file,body in contents.items():
       h=base64.urlsafe_b64encode(hashlib.sha256(body).digest()).decode().rstrip("=")
       rows.append([file,"sha256="+h,str(len(body))])
    rows.append([prefix+"RECORD","",""])
    stream=io.StringIO()
    csv.writer(stream,lineterminator="\n").writerows(rows)
    contents[prefix+"RECORD"]=stream.getvalue().encode()
    buf=io.BytesIO()
    with zipfile.ZipFile(buf,"w",zipfile.ZIP_STORED) as z:
        for file,body in contents.items():z.writestr(file,body)
    return buf.getvalue()

class AcquisitionTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix="deus_wheels_no_network_")
        self.addCleanup(self.tmp.cleanup)
        self.base=Path(self.tmp.name)
        self.stage=self.base/"stage"
        self.stage.mkdir()
        self.dest=self.base/"Wheelhouse"
        self.wheels={}
        lock=[]
        for name,version,filename,tag,requires in PACKAGES:
            blob=fixture(name,version,tag,requires)
            self.wheels[name]=(blob,version,filename)
            lock.append({"name":name,"version":version,"filename":filename,"sha256":sha(blob)})
        selection={"schema":"DEUS_A05_WINDOWS_PYTHON314_REGULAR_WHEEL_SELECTION_V1",
                   "owner_windows_inventory":{"target_wheel_abi":"cp314","gil_disabled":False},
                   "selected_wheels":lock}
        (self.stage/"a05_windows_python314_regular_wheel_selection_v1.json").write_text(
            json.dumps(selection),encoding="utf-8")
        self.fault=""
    def fake(self,url,limit,output=None):
        if output is None:
            name=url.split("/")[4]
            body,version,filename=self.wheels[name]
            value={"info":{"name":name,"version":version},"urls":[{
                "filename":filename,"packagetype":"bdist_wheel","yanked":False,
                "size":len(body),"digests":{"sha256":sha(body)},
                "url":"https://files.pythonhosted.org/packages/mock/"+filename
            }]}
            if self.fault=="bad_metadata_sha" and name=="cffi":
                value["urls"][0]["digests"]["sha256"]="0"*64
            raw=json.dumps(value).encode()
            return raw,len(raw),sha(raw)
        filename=url.split("/")[-1]
        name=next(p[0] for p in PACKAGES if p[2]==filename)
        body=self.wheels[name][0]
        if self.fault=="incomplete" and name=="cffi":
            body=body[:-15]
        output.write(body)
        return None,len(body),sha(body)
    def test_three_fake_actual_wheels_retained_in_upload_zip(self):
        with mock.patch.object(dl,"_fetch",self.fake):
            receipt=dl.retrieve_three(self.stage,self.dest)
        self.assertEqual(receipt["downloaded_files"],3)
        self.assertEqual(receipt["installed_files"],0)
        self.assertTrue(receipt["no_python_environment_change"])
        zipped=self.dest/receipt["wheelhouse_archive_basename"]
        self.assertEqual(sha(zipped.read_bytes()),receipt["wheelhouse_archive_sha256"])
        with zipfile.ZipFile(zipped) as z:
            self.assertEqual(z.testzip(),None)
            self.assertEqual(len(z.namelist()),4)
            for name,(body,_,filename) in self.wheels.items():
                self.assertEqual(z.read(filename),body)
            self.assertEqual(len(z.read("SHA256SUMS.txt").splitlines()),3)

    def test_published_sha_mismatch_no_output_archive(self):
        self.fault="bad_metadata_sha"
        with mock.patch.object(dl,"_fetch",self.fake):
            with self.assertRaisesRegex(dl.DownloadStop,"PYPI_PUBLISHED_SHA_MISMATCH"):
                dl.retrieve_three(self.stage,self.dest)
        self.assertFalse(list(self.dest.glob("*.zip")))

    def test_incomplete_download_no_output_archive(self):
        self.fault="incomplete"
        with mock.patch.object(dl,"_fetch",self.fake):
            with self.assertRaisesRegex(dl.DownloadStop,"WHEEL_PYPI_SIZE_MISMATCH"):
                dl.retrieve_three(self.stage,self.dest)
        self.assertFalse(list(self.dest.glob("*.zip")))

    def test_untrusted_cdn_host_refused(self):
        name,version,filename,tag,req=PACKAGES[0]
        meta={"info":{"name":name,"version":version},"urls":[{
            "filename":filename,"packagetype":"bdist_wheel","yanked":False,
            "size":1400,"digests":{"sha256":"A"*64},
            "url":"https://evil.example/packages/"+filename
        }]}
        with self.assertRaisesRegex(dl.DownloadStop,"PYPI_FILE_URL_WRONG_HOST"):
            dl.select_exact_pypi_url(meta,name,version,filename,"A"*64)

    def test_yanked_even_without_reason_is_forbidden(self):
        name,version,filename,tag,req=PACKAGES[0]
        body=self.wheels[name][0]
        meta={"info":{"name":name,"version":version},"urls":[{
            "filename":filename,"packagetype":"bdist_wheel","yanked":True,
            "yanked_reason":None,"size":len(body),
            "digests":{"sha256":sha(body)},
            "url":"https://files.pythonhosted.org/packages/mock/"+filename
        }]}
        with self.assertRaisesRegex(dl.DownloadStop,"YANKED_PYPI_WHEEL"):
            dl.select_exact_pypi_url(meta,name,version,filename,sha(body))

    def test_redirect_http_handler_fails_closed(self):
        handler=dl.NoRedirect()
        with self.assertRaisesRegex(dl.DownloadStop,"UNEXPECTED_HTTPS_REDIRECT"):
            handler.redirect_request(None,None,302,"x",None,"https://wrong.example")

    def test_http_non_tls_refused_before_transport(self):
        with self.assertRaisesRegex(dl.DownloadStop,"OFFICIAL_PYPI_METADATA_URL_ONLY"):
            dl._fetch("http://pypi.org/pypi/cffi/2.1.1/json",2048)

    def test_extra_wheel_forbidden(self):
        selection=json.loads((self.stage/"a05_windows_python314_regular_wheel_selection_v1.json").read_text())
        selection["selected_wheels"].append(selection["selected_wheels"][0])
        (self.stage/"a05_windows_python314_regular_wheel_selection_v1.json").write_text(json.dumps(selection))
        with self.assertRaisesRegex(dl.DownloadStop,"EXACT_THREE_WHEELS_REQUIRED"):
            dl.retrieve_three(self.stage,self.dest)

if __name__=="__main__":
    unittest.main(verbosity=2)
