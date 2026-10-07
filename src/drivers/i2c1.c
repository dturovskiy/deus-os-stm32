#include <stdint.h>
#include "drivers/i2c1.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_APB2ENR      REG32(0x40021018u)
#define RCC_APB1RSTR     REG32(0x40021010u)
#define RCC_APB1ENR      REG32(0x4002101Cu)
#define GPIOB_CRL        REG32(0x40010C00u)
#define I2C1_CR1         REG32(0x40005400u)
#define I2C1_CR2         REG32(0x40005404u)
#define I2C1_DR          REG32(0x40005410u)
#define I2C1_SR1         REG32(0x40005414u)
#define I2C1_SR2         REG32(0x40005418u)
#define I2C1_CCR         REG32(0x4000541Cu)
#define I2C1_TRISE       REG32(0x40005420u)
#define RCC_APB2ENR_IOPBEN   (1u << 3)
#define RCC_APB1ENR_I2C1EN   (1u << 21)
#define RCC_APB1RSTR_I2C1RST (1u << 21)
#define I2C_CR1_PE        (1u << 0)
#define I2C_CR1_START     (1u << 8)
#define I2C_CR1_STOP      (1u << 9)
#define I2C_SR1_SB        (1u << 0)
#define I2C_SR1_ADDR      (1u << 1)
#define I2C_SR1_BTF       (1u << 2)
#define I2C_SR1_TXE       (1u << 7)
#define I2C_SR1_BERR      (1u << 8)
#define I2C_SR1_ARLO      (1u << 9)
#define I2C_SR1_AF        (1u << 10)
#define I2C_SR2_BUSY      (1u << 1)
#define I2C1_PCLK_MHZ     36u
#define I2C1_CCR_100KHZ   180u
#define I2C1_TRISE_100KHZ 37u
#define I2C_SPIN_LIMIT    100000u

void i2c1_init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB_CRL &= ~((0xFu << 24) | (0xFu << 28));
    GPIOB_CRL |= ((0xFu << 24) | (0xFu << 28));

    RCC_APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC_APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    I2C1_CR1 = 0u;
    I2C1_CR2 = I2C1_PCLK_MHZ;
    I2C1_CCR = I2C1_CCR_100KHZ;
    I2C1_TRISE = I2C1_TRISE_100KHZ;
    I2C1_CR1 = I2C_CR1_PE;
}

static int i2c1_wait_bus_free(void)
{
    uint32_t spins = I2C_SPIN_LIMIT;

    while ((I2C1_SR2 & I2C_SR2_BUSY) != 0u)
    {
        if (spins == 0u)
        {
            return 0;
        }
        --spins;
    }

    return 1;
}

int i2c1_probe(uint8_t address)
{
    uint32_t spins;
    uint32_t sr1;

    if (i2c1_wait_bus_free() == 0)
    {
        return -1;
    }

    I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF);
    I2C1_CR1 |= I2C_CR1_START;
    spins = I2C_SPIN_LIMIT;

    while ((I2C1_SR1 & I2C_SR1_SB) == 0u)
    {
        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return -1;
        }
        --spins;
    }

    I2C1_DR = ((uint32_t)address << 1);
    spins = I2C_SPIN_LIMIT;

    for (;;)
    {
        sr1 = I2C1_SR1;

        if ((sr1 & I2C_SR1_ADDR) != 0u)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            I2C1_CR1 |= I2C_CR1_STOP;
            return 1;
        }

        if ((sr1 & I2C_SR1_AF) != 0u)
        {
            I2C1_SR1 &= ~I2C_SR1_AF;
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        if ((sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
        {
            I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO);
            I2C1_CR1 |= I2C_CR1_STOP;
            return -1;
        }

        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return -1;
        }
        --spins;
    }
}

int i2c1_write(uint8_t address, const uint8_t *data, uint32_t length)
{
    uint32_t spins;
    uint32_t sr1;
    uint32_t index;

    if ((data == (const uint8_t *)0) || (length == 0u))
    {
        return 0;
    }

    if (i2c1_wait_bus_free() == 0)
    {
        return 0;
    }

    I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF);
    I2C1_CR1 |= I2C_CR1_START;
    spins = I2C_SPIN_LIMIT;

    while ((I2C1_SR1 & I2C_SR1_SB) == 0u)
    {
        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }
        --spins;
    }

    I2C1_DR = ((uint32_t)address << 1);
    spins = I2C_SPIN_LIMIT;

    for (;;)
    {
        sr1 = I2C1_SR1;

        if ((sr1 & I2C_SR1_ADDR) != 0u)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            break;
        }

        if ((sr1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
        {
            I2C1_SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO);
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }
        --spins;
    }

    for (index = 0u; index < length; ++index)
    {
        spins = I2C_SPIN_LIMIT;
        for (;;)
        {
            sr1 = I2C1_SR1;

            if ((sr1 & I2C_SR1_TXE) != 0u)
            {
                break;
            }

            if ((sr1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
            {
                I2C1_SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO);
                I2C1_CR1 |= I2C_CR1_STOP;
                return 0;
            }

            if (spins == 0u)
            {
                I2C1_CR1 |= I2C_CR1_STOP;
                return 0;
            }
            --spins;
        }

        I2C1_DR = data[index];
    }

    spins = I2C_SPIN_LIMIT;
    for (;;)
    {
        sr1 = I2C1_SR1;

        if ((sr1 & I2C_SR1_BTF) != 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 1;
        }

        if ((sr1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
        {
            I2C1_SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO);
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }
        --spins;
    }
}
