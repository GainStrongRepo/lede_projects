/*
 * CPE5GD02 reference board support
 *
 * Copyright (c) 2013 The Linux Foundation. All rights reserved.
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"


#define CPE5GD02_GPIO_LED_WIFI_2G               1
#define CPE5GD02_GPIO_LED_WAN                   2
#define CPE5GD02_GPIO_LED_LAN                   3
#define CPE5GD02_GPIO_LED_WIFI_2G_ANT0          11
#define CPE5GD02_GPIO_LED_WIFI_5G_D3            12
#define CPE5GD02_GPIO_LED_WIFI_2G_ANT1          13
#define CPE5GD02_GPIO_LED_SYSTEM                14
#define CPE5GD02_GPIO_LED_WIFI_5G_D5            15
#define CPE5GD02_GPIO_LED_WIFI_5G_D4            16
#define CPE5GD02_GPIO_LED_WIFI_5G_D2            17

#define CPE5GD02_GPIO_BTN_RST                   4

#define CPE5GD02_KEYS_POLL_INTERVAL             20  /* msecs */
#define CPE5GD02_KEYS_DEBOUNCE_INTERVAL         (3 * CPE5GD02_KEYS_POLL_INTERVAL)

#define CPE5GD02_MAC0_OFFSET                    0
#define CPE5GD02_MAC1_OFFSET                    6
#define CPE5GD02_WMAC_CALDATA_OFFSET            0x1000

static const char *cpe5gd02_part_probes[] = {
        "tp-link",
        NULL,
};

static struct flash_platform_data cpe5gd02_flash_data = {
        .part_probes    = cpe5gd02_part_probes,
};

static struct gpio_led cpe5gd02_leds_gpio[] __initdata = {
        {
                .name       = "wifi-2g",
                .gpio       = CPE5GD02_GPIO_LED_WIFI_2G,
                .active_low = 1,
        },
        {
                .name       = "wan",
                .gpio       = CPE5GD02_GPIO_LED_WAN,
                .active_low = 1,
        },
        {
                .name       = "lan",
                .gpio       = CPE5GD02_GPIO_LED_LAN,
                .active_low = 1,
        },
        {
                .name       = "wifi-2g-ant0",
                .gpio       = CPE5GD02_GPIO_LED_WIFI_2G_ANT0,
                .active_low = 1,
        },
        {
                .name       = "wifi-5g-d3",
                .gpio       = CPE5GD02_GPIO_LED_WIFI_5G_D3,
                .active_low = 1,
        },
        {
                .name       = "wifi-2g-ant1",
                .gpio       = CPE5GD02_GPIO_LED_WIFI_2G_ANT1,
                .active_low = 1,
        },
        {
                .name       = "system",
                .gpio       = CPE5GD02_GPIO_LED_SYSTEM,
                .active_low = 1,
        },
        {
                .name       = "wifi-5g-d5",
                .gpio       = CPE5GD02_GPIO_LED_WIFI_5G_D5,
                .active_low = 1,
        },
        {
                .name       = "wifi-5g-d4",
                .gpio       = CPE5GD02_GPIO_LED_WIFI_5G_D4,
                .active_low = 1,
        },
        {
                .name       = "wifi-5g-d2",
                .gpio       = CPE5GD02_GPIO_LED_WIFI_5G_D2,
                .active_low = 1,
        },
};

static struct gpio_keys_button cpe5gd02_gpio_keys[] __initdata = {
        {
                .desc       = "reset button",
                .type       = EV_KEY,
                .code       = KEY_RESTART,
                .debounce_interval = CPE5GD02_KEYS_DEBOUNCE_INTERVAL,
                .gpio       = CPE5GD02_GPIO_BTN_RST,
                .active_low = 1,
        },
};

static void __init cpe5gd02_gpio_led_setup(void)
{
        unsigned int old_func, new_func;
        void __iomem *QCA9531_GPIO_FUNC = ioremap_nocache(AR71XX_GPIO_BASE + 0x6c, 0x04);
        // disable observation of CPU_CLK/4
        old_func = __raw_readl(QCA9531_GPIO_FUNC);
        new_func = old_func & (~(1 << 7));
        __raw_writel(new_func, QCA9531_GPIO_FUNC);
        iounmap(QCA9531_GPIO_FUNC);

        ath79_register_leds_gpio(-1, ARRAY_SIZE(cpe5gd02_leds_gpio), cpe5gd02_leds_gpio);
        ath79_register_gpio_keys_polled(-1, CPE5GD02_KEYS_POLL_INTERVAL,
                ARRAY_SIZE(cpe5gd02_gpio_keys), cpe5gd02_gpio_keys);
}

static void __init cpe5gd02_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&cpe5gd02_flash_data);
	cpe5gd02_gpio_led_setup();

        ath79_register_usb();
        ath79_register_pci();

	ath79_register_wmac(art + CPE5GD02_WMAC_CALDATA_OFFSET, NULL);
	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + CPE5GD02_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + CPE5GD02_MAC1_OFFSET, 0);

	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_CPE5GD02, "CPE5GD02", "CPE5GD02 board", cpe5gd02_setup);
