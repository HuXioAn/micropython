/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"

#if MICROPY_PY_NETWORK

#include "shared/netutils/netutils.h"
#include "modnetwork.h"

#if MICROPY_PY_LWIP
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/apps/mdns.h"
#endif

#if MICROPY_PY_NETWORK_CYW43 && MICROPY_PY_NETWORK_CYW43_USE_LIB_DRIVER
// So that CYW43_LINK_xxx constants are available to MICROPY_PORT_NETWORK_INTERFACES.
#include "lib/cyw43-driver/src/cyw43.h"
#endif

/// \module network - network configuration
///
/// This module provides network drivers and routing configuration.

char mod_network_country_code[2] = "XX";

#ifndef MICROPY_PY_NETWORK_HOSTNAME_DEFAULT
#error "MICROPY_PY_NETWORK_HOSTNAME_DEFAULT must be set in mpconfigport.h or mpconfigboard.h"
#endif

char mod_network_hostname[MICROPY_PY_NETWORK_HOSTNAME_MAX_LEN] = MICROPY_PY_NETWORK_HOSTNAME_DEFAULT;

void mod_network_init(void) {
    mp_obj_list_init(&MP_STATE_PORT(mod_network_nic_list), 0);
}

void mod_network_deinit(void) {
}

void mod_network_register_nic(mp_obj_t nic) {
    for (mp_uint_t i = 0; i < MP_STATE_PORT(mod_network_nic_list).len; i++) {
        if (MP_STATE_PORT(mod_network_nic_list).items[i] == nic) {
            // nic already registered
            return;
        }
    }
    // nic not registered so add to list
    mp_obj_list_append(MP_OBJ_FROM_PTR(&MP_STATE_PORT(mod_network_nic_list)), nic);
}

mp_obj_t mod_network_find_nic(const uint8_t *ip) {
    // find a NIC that is suited to given IP address
    for (mp_uint_t i = 0; i < MP_STATE_PORT(mod_network_nic_list).len; i++) {
        mp_obj_t nic = MP_STATE_PORT(mod_network_nic_list).items[i];
        // TODO check IP suitability here
        // mod_network_nic_protocol_t *nic_protocol = (mod_network_nic_protocol_t *)MP_OBJ_TYPE_GET_SLOT(mp_obj_get_type(nic), protocol);
        return nic;
    }

    mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("no available NIC"));
}

STATIC mp_obj_t network_route(void) {
    return MP_OBJ_FROM_PTR(&MP_STATE_PORT(mod_network_nic_list));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(network_route_obj, network_route);

STATIC mp_obj_t network_country(size_t n_args, const mp_obj_t *args) {
    if (n_args == 0) {
        return mp_obj_new_str(mod_network_country_code, 2);
    } else {
        size_t len;
        const char *str = mp_obj_str_get_data(args[0], &len);
        if (len != 2) {
            mp_raise_ValueError(NULL);
        }
        mod_network_country_code[0] = str[0];
        mod_network_country_code[1] = str[1];
        return mp_const_none;
    }
}
// TODO: Non-static to allow backwards-compatible pyb.country.
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_network_country_obj, 0, 1, network_country);

STATIC mp_obj_t network_hostname(size_t n_args, const mp_obj_t *args) {
    if (n_args == 0) {
        return mp_obj_new_str(mod_network_hostname, strlen(mod_network_hostname));
    } else {
        size_t len;
        const char *str = mp_obj_str_get_data(args[0], &len);
        if (len >= MICROPY_PY_NETWORK_HOSTNAME_MAX_LEN) {
            mp_raise_ValueError(NULL);
        }
        strcpy(mod_network_hostname, str);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_network_hostname_obj, 0, 1, network_hostname);

STATIC const mp_rom_map_elem_t mp_module_network_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_network) },
    { MP_ROM_QSTR(MP_QSTR_route), MP_ROM_PTR(&network_route_obj) },
    { MP_ROM_QSTR(MP_QSTR_country), MP_ROM_PTR(&mod_network_country_obj) },
    { MP_ROM_QSTR(MP_QSTR_hostname), MP_ROM_PTR(&mod_network_hostname_obj) },

    // Defined per port in mpconfigport.h
    MICROPY_PORT_NETWORK_INTERFACES

    // Constants
    { MP_ROM_QSTR(MP_QSTR_STA_IF), MP_ROM_INT(MOD_NETWORK_STA_IF) },
    { MP_ROM_QSTR(MP_QSTR_AP_IF), MP_ROM_INT(MOD_NETWORK_AP_IF) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_network_globals, mp_module_network_globals_table);

const mp_obj_module_t mp_module_network = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_network_globals,
};

MP_REGISTER_MODULE(MP_QSTR_network, mp_module_network);

/*******************************************************************************/
// Implementations of network methods that can be used by any interface

#if MICROPY_PY_LWIP

mp_obj_t mod_network_nic_ifconfig(struct netif *netif, size_t n_args, const mp_obj_t *args) {
    if (n_args == 0) {
        // Get IP addresses
        const ip_addr_t *dns = dns_getserver(0);
        mp_obj_t tuple[4] = {
            netutils_format_ipv4_addr((uint8_t *)&netif->ip_addr, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t *)&netif->netmask, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t *)&netif->gw, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t *)dns, NETUTILS_BIG),
        };
        return mp_obj_new_tuple(4, tuple);
    } else if (args[0] == MP_OBJ_NEW_QSTR(MP_QSTR_dhcp)) {
        // Start the DHCP client
        if (dhcp_supplied_address(netif)) {
            dhcp_renew(netif);
        } else {
            dhcp_stop(netif);
            dhcp_start(netif);
        }

        // Wait for DHCP to get IP address
        uint32_t start = mp_hal_ticks_ms();
        while (!dhcp_supplied_address(netif)) {
            if (mp_hal_ticks_ms() - start > 10000) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("timeout waiting for DHCP to get IP address"));
            }
            mp_hal_delay_ms(100);
        }

        return mp_const_none;
    } else {
        // Release and stop any existing DHCP
        dhcp_release(netif);
        dhcp_stop(netif);
        // Set static IP addresses
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[0], 4, &items);
        netutils_parse_ipv4_addr(items[0], (uint8_t *)&netif->ip_addr, NETUTILS_BIG);
        netutils_parse_ipv4_addr(items[1], (uint8_t *)&netif->netmask, NETUTILS_BIG);
        netutils_parse_ipv4_addr(items[2], (uint8_t *)&netif->gw, NETUTILS_BIG);
        ip_addr_t dns;
        netutils_parse_ipv4_addr(items[3], (uint8_t *)&dns, NETUTILS_BIG);
        dns_setserver(0, &dns);
        return mp_const_none;
    }
}

#endif

MP_REGISTER_ROOT_POINTER(mp_obj_list_t mod_network_nic_list);

#endif  // MICROPY_PY_NETWORK
