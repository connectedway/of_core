/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/handle.h"
#include "ofc/libc.h"
#include "ofc/net.h"
#include "ofc/lock.h"
#include "ofc/queue.h"
#include "ofc/event.h"
#include "ofc/thread.h"
#include "ofc/path.h"

#include "ofc/heap.h"

#if defined(OFC_PERSIST)

#include "ofc/dom.h"

#endif

#include "ofc/persist.h"

#include "ofc/file.h"

#if defined(OFC_PERSIST)
static OFC_CHAR *strmode[OFC_CONFIG_MODE_MAX] =
        {
                "BMODE",
                "PMODE",
                "MMODE",
                "HMODE"
        };

static OFC_CHAR *striconfigtype[OFC_CONFIG_ICONFIG_NUM] =
        {
                "auto",
                "manual"
        };
#endif

/*
 * Who to notify on configuration events
 */
static OFC_HANDLE event_queue;

typedef struct {
    OFC_UINT16 num_dns;
    OFC_IPADDR *dns;        /* This structure can be allocated */
    /* dynamically with a larger array bound */
} DNSLIST;

/**
 * Network Configuration
 *
 * This defines the format of the interface configuration that we maintain
 */
typedef struct {
    OFC_CONFIG_MODE netbios_mode;
    OFC_IPADDR ipaddress;
    OFC_IPADDR bcast;
    OFC_IPADDR mask;
    OFC_VOID *private;
    OFC_UINT16 num_wins;
    OFC_IPADDR *winslist;    /* This structure can be allocated */
    /* dynamically with a larger array bound */
    OFC_CHAR *master;
} OFC_CONFIG_ICONFIG;

typedef struct {
    OFC_LOCK *config_lock;
    OFC_BOOL log_console;
    OFC_UINT log_level;
    OFC_TCHAR *workstation_name;
    OFC_TCHAR *workstation_domain;
    OFC_TCHAR *workstation_desc;
    OFC_CONFIG_ICONFIG_TYPE iconfig_type;
    OFC_UINT16 interface_count;
    OFC_UINT16 ipv4_interface_count;
    OFC_CONFIG_ICONFIG *interface_config;
    DNSLIST netbios_dns;

    OFC_BOOL enableAutoIP;
    OFC_BOOL netbiosEnabled;
    OFC_BOOL loaded;

    OFC_UUID uuid;
    OFC_CHAR *default_realm;
    OFC_UINT32 update_count;
    OFC_HANDLE subconfigs;
} OFC_CONFIG;

static OFC_VOID ofc_persist_lock(OFC_VOID);

static OFC_VOID ofc_persist_unlock(OFC_VOID);

static OFC_CONFIG *g_config = NULL;

static OFC_CORE_LIB OFC_VOID
ofc_set_config(OFC_CONFIG *configp) {
    g_config = configp;
}

static OFC_CORE_LIB OFC_CONFIG *
ofc_get_config(OFC_VOID) {
    OFC_CONFIG *ret;

    ret = g_config;
    return (ret);
}

#if defined(OFC_PERSIST)

static OFC_DOMDocument *ofc_persist_make_dom(OFC_VOID);

static OFC_VOID ofc_persist_parse_dom(OFC_DOMNode *config_dom);

static OFC_DOMDocument *
ofc_persist_make_dom(OFC_VOID) {
    OFC_DOMDocument *doc;
    OFC_DOMNode *node;
    OFC_DOMNode *config_node;
    OFC_DOMNode *ip_node;
    OFC_DOMNode *interfaces_node;
    OFC_DOMNode *interface_node;
    OFC_DOMNode *dns_node;
    OFC_DOMNode *drives_node;
    OFC_DOMNode *wins_node;
    OFC_DOMNode *map_node;
    OFC_DOMNode *log_node;
    OFC_BOOL error_state;
    OFC_CHAR *cstr;
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_IPADDR *iparray;
    OFC_INT i;
    OFC_INT j;
    OFC_CONFIG *ofc_persist;
    OFC_CHAR ip_addr[IP6STR_LEN];
    PERSIST_REGISTER *subconfig;
    OFC_DOMNode *sub_node;

    doc = OFC_NULL;
    ofc_persist = ofc_get_config();

    if (ofc_persist != OFC_NULL) {
        error_state = OFC_FALSE;

        doc = ofc_dom_create_document(OFC_NULL, OFC_NULL, OFC_NULL);
        if (doc != OFC_NULL) {
            node = ofc_dom_create_processing_instruction
                    (doc, "xml", "version=\"1.0\" encoding=\"utf-8\"");
            if (node != OFC_NULL)
                ofc_dom_append_child(doc, node);
            else
                error_state = OFC_TRUE;
        }

        config_node = OFC_NULL;
        if (!error_state) {
            config_node = ofc_dom_create_element(doc, "of_core");
            if (config_node != OFC_NULL) {
                ofc_dom_set_attribute(config_node, "version", "0.0");
                ofc_dom_append_child(doc, config_node);
            } else
                error_state = OFC_TRUE;
        }

        if (!error_state)
          {
            log_node = ofc_dom_create_element(doc, "logging");
            if (log_node != OFC_NULL)
              {
                ofc_dom_append_child(config_node, log_node);
              }
            else
              error_state = OFC_TRUE;
          }

        if (!error_state)
          {
            node =
              ofc_dom_create_element_cdata(doc, "console", 
                                           ofc_persist->log_console ==
                                           OFC_TRUE ? "yes" : "no");
            if (node != OFC_NULL)
              {
                ofc_dom_append_child(log_node, node);
              }
            else
              error_state = OFC_TRUE;
          }

        if (!error_state)
          {
            OFC_CHAR *level = ofc_saprintf("%d", ofc_persist->log_level);
            node =
              ofc_dom_create_element_cdata(doc, "level", level);
            if (node != OFC_NULL)
              {
                ofc_dom_append_child(log_node, node);
              }
            else
              error_state = OFC_TRUE;
            ofc_free(level);
          }

        if (!error_state) {
            cstr =
                    ofc_tstr2cstr(ofc_persist->workstation_name);
            node = ofc_dom_create_element_cdata(doc, "devicename", cstr);
            ofc_free(cstr);
            if (node != OFC_NULL)
                ofc_dom_append_child(config_node, node);
            else
                error_state = OFC_TRUE;
        }

        if (!error_state) {
            cstr = ofc_malloc(UUID_STR_LEN + 1);
            ofc_uuidtoa(ofc_persist->uuid, cstr);
            node = ofc_dom_create_element_cdata(doc, "uuid", cstr);
            ofc_free(cstr);
            if (node != OFC_NULL)
                ofc_dom_append_child(config_node, node);
            else
                error_state = OFC_TRUE;
        }

        if (!error_state && ofc_persist->default_realm != OFC_NULL) {
          node = ofc_dom_create_element_cdata(doc,
                                              "realm",
                                              ofc_persist->default_realm);
          if (node != OFC_NULL)
            ofc_dom_append_child(config_node, node);
          else
            error_state = OFC_TRUE;
        }

	if (!error_state) {
            cstr =
                    ofc_tstr2cstr(ofc_persist->workstation_desc);
            node =
                    ofc_dom_create_element_cdata(doc, "description", cstr);
            ofc_free(cstr);
            if (node != OFC_NULL)
                ofc_dom_append_child(config_node, node);
            else
                error_state = OFC_TRUE;
        }

        ip_node = OFC_NULL;
        if (!error_state) {
            ip_node = ofc_dom_create_element(doc, "ip");
            if (ip_node != OFC_NULL)
                ofc_dom_append_child(config_node, ip_node);
            else
                error_state = OFC_TRUE;
        }

        if (!error_state) {
            node = ofc_dom_create_element_cdata(doc, "autoip",
                                                ofc_persist->enableAutoIP ?
                                                "yes" : "no");
            if (node != OFC_NULL)
                ofc_dom_append_child(ip_node, node);
            else
                error_state = OFC_TRUE;
        }

        if (!error_state) {
            node = ofc_dom_create_element_cdata(doc, "netbios",
                                                ofc_persist->netbiosEnabled ?
                                                "yes" : "no");
            if (node != OFC_NULL)
                ofc_dom_append_child(ip_node, node);
            else
                error_state = OFC_TRUE;
        }

        interfaces_node = OFC_NULL;
        if (!error_state) {
            interfaces_node = ofc_dom_create_element(doc, "interfaces");

            if (interfaces_node != OFC_NULL)
                ofc_dom_append_child(ip_node, interfaces_node);
            else
                error_state = OFC_TRUE;
        }

        if (!error_state) {
            if (ofc_persist->iconfig_type < OFC_CONFIG_ICONFIG_NUM) {
                node =
                        ofc_dom_create_element_cdata(doc, "config",
                                                     striconfigtype
                                                     [ofc_persist->iconfig_type]);
            } else {
                node =
                        ofc_dom_create_element_cdata(doc, "config",
                                                     striconfigtype
                                                     [OFC_CONFIG_ICONFIG_AUTO]);
            }

            if (node != OFC_NULL)
                ofc_dom_append_child(interfaces_node, node);
            else
                error_state = OFC_TRUE;
        }

        if (ofc_persist->iconfig_type != OFC_CONFIG_ICONFIG_AUTO) {
            for (i = 0;
                 !error_state && i < ofc_persist->interface_count;
                 i++) {
                interface_node = ofc_dom_create_element(doc, "interface");

                if (interface_node != OFC_NULL) {
                    ofc_dom_append_child(interfaces_node, interface_node);

                    interface_config = ofc_persist->interface_config;

                    node = ofc_dom_create_element_cdata
                            (doc, "ipaddress",
                             ofc_ntop(&interface_config[i].ipaddress,
                                      ip_addr, IP6STR_LEN));

                    if (node != OFC_NULL)
                        ofc_dom_append_child(interface_node, node);
                    else
                        error_state = OFC_TRUE;

                    if (!error_state) {
                        node = ofc_dom_create_element_cdata
                                (doc, "bcast",
                                 ofc_ntop(&interface_config[i].bcast,
                                          ip_addr, IP6STR_LEN));

                        if (node != OFC_NULL)
                            ofc_dom_append_child(interface_node, node);
                        else
                            error_state = OFC_TRUE;
                    }

                    if (!error_state) {
                        node = ofc_dom_create_element_cdata
                                (doc, "mask",
                                 ofc_ntop(&interface_config[i].mask,
                                          ip_addr, IP6STR_LEN));

                        if (node != OFC_NULL)
                            ofc_dom_append_child(interface_node, node);
                        else
                            error_state = OFC_TRUE;
                    }

                    if (!error_state) {
                        node = ofc_dom_create_element_cdata
                                (doc, "mode",
                                 strmode[interface_config[i].netbios_mode]);

                        if (node != OFC_NULL)
                            ofc_dom_append_child(interface_node, node);
                        else
                            error_state = OFC_TRUE;
                    }

                    if (!error_state) {
                        wins_node = OFC_NULL;
                        wins_node = ofc_dom_create_element(doc, "winslist");
                        if (wins_node != OFC_NULL)
                            ofc_dom_append_child(interface_node, wins_node);
                        else
                            error_state = OFC_TRUE;

                        iparray = interface_config[i].winslist;
                        for (j = 0;
                             j < interface_config[i].num_wins &&
                             !error_state;
                             j++) {
                            node = ofc_dom_create_element_cdata
                                    (doc, "wins",
                                     ofc_ntop(&iparray[j], ip_addr, IP6STR_LEN));

                            if (node != OFC_NULL)
                                ofc_dom_append_child(wins_node, node);
                            else
                                error_state = OFC_TRUE;
                        }
                    }

                    if (!error_state) {
                        node = ofc_dom_create_element_cdata
                                (doc, "master", interface_config[i].master);

                        if (node != OFC_NULL)
                            ofc_dom_append_child(interface_node, node);
                        else
                            error_state = OFC_TRUE;
                    }
                }
            }
        }

        dns_node = OFC_NULL;
        if (!error_state) {
            dns_node = ofc_dom_create_element(doc, "dnslist");
            if (dns_node != OFC_NULL)
                ofc_dom_append_child(ip_node, dns_node);
            else
                error_state = OFC_TRUE;
        }

        for (i = 0;
             !error_state && i < ofc_persist->netbios_dns.num_dns;
             i++) {
            iparray = ofc_persist->netbios_dns.dns;
            node = ofc_dom_create_element_cdata
                    (doc, "dns", ofc_ntop(&iparray[i], ip_addr, IP6STR_LEN));
            if (node != OFC_NULL)
                ofc_dom_append_child(dns_node, node);
            else
                error_state = OFC_TRUE;
        }

        if (!error_state) {
            OFC_CTCHAR *prefix;
            OFC_CTCHAR *desc;
            OFC_PATH *path;
            OFC_BOOL thumbnail;

            drives_node = ofc_dom_create_element(doc, "drives");
            if (drives_node != OFC_NULL)
                ofc_dom_append_child(config_node, drives_node);
            else
                error_state = OFC_TRUE;

            for (i = 0; i < OFC_MAX_MAPS && !error_state; i++) {
                ofc_path_get_mapW(i, &prefix, &desc, &path, &thumbnail);
                if (prefix != OFC_NULL) {
                    map_node = ofc_dom_create_element(doc, "map");
                    if (map_node != OFC_NULL)
                        ofc_dom_append_child(drives_node, map_node);
                    else
                        error_state = OFC_TRUE;

                    if (!error_state) {
                        OFC_CHAR *cprefix;
                        cprefix = ofc_tstr2cstr(prefix);
                        node = ofc_dom_create_element_cdata
                                (doc, "drive", cprefix);
                        if (node != OFC_NULL)
                            ofc_dom_append_child(map_node, node);
                        else
                            error_state = OFC_TRUE;
                        ofc_free(cprefix);
                    }

                    if (!error_state) {
                        OFC_CHAR *cdesc;
                        cdesc = ofc_tstr2cstr(desc);
                        node = ofc_dom_create_element_cdata
                                (doc, "description", cdesc);
                        if (node != OFC_NULL)
                            ofc_dom_append_child(map_node, node);
                        else
                            error_state = OFC_TRUE;
                        ofc_free(cdesc);
                    }

                    if (!error_state) {
                        OFC_SIZET rem;
                        OFC_SIZET size;
                        OFC_CHAR *filename;

                        rem = 0;
                        size = ofc_path_printA(path, OFC_NULL, &rem);
                        filename = ofc_malloc(size + 1);

                        rem = size + 1;
                        ofc_path_printA(path, &filename, &rem);
                        node = ofc_dom_create_element_cdata(doc, "path", filename);
                        ofc_free(filename);

                        if (node != OFC_NULL)
                            ofc_dom_append_child(map_node, node);
                        else
                            error_state = OFC_TRUE;
                    }

                    if (!error_state) {
                        node = ofc_dom_create_element_cdata(doc, "thumbnail",
                                                            thumbnail ?
                                                            "yes" : "no");
                        if (node != OFC_NULL)
                            ofc_dom_append_child(map_node, node);
                        else
                            error_state = OFC_TRUE;
                    }
                }
            }
        }

	for (subconfig = ofc_queue_first(ofc_persist->subconfigs);
	     subconfig != OFC_NULL && !error_state;
	     subconfig = ofc_queue_next(ofc_persist->subconfigs, subconfig))
	  {
            sub_node = ofc_dom_create_element(doc, subconfig->tag);
            if (sub_node != OFC_NULL)
	      {
		error_state = (*subconfig->make)(doc, sub_node);
		if (error_state)
		  {
		    ofc_dom_destroy_node(sub_node);
		  }
		else
		  {
		    ofc_dom_append_child(config_node, sub_node);
		  }
	      }
	  }

        if (error_state && doc != OFC_NULL) {
            ofc_dom_destroy_document(doc);
            doc = OFC_NULL;
        }
    }
    return (doc);
}

static OFC_VOID
ofc_persist_parse_dom(OFC_DOMNode *config_dom) {
    OFC_BOOL error_state;
    OFC_DOMNode *config_node;
    OFC_DOMNode *ip_node;
    OFC_DOMNode *interfaces_node;
    OFC_DOMNode *dns_node;
    OFC_DOMNodelist *dns_nodelist;
    OFC_DOMNode *wins_node;
    OFC_DOMNodelist *wins_nodelist;
    OFC_DOMNode *drives_node;
    OFC_DOMNodelist *drives_nodelist;
    OFC_DOMNode *map_node;
    OFC_DOMNode *log_node;
    OFC_DOMNodelist *interface_nodelist;
    OFC_CHAR *version;
    OFC_CHAR *value;
    OFC_INT i;
    OFC_INT j;
    OFC_INT num_wins;
    OFC_IPADDR ipaddress;
    OFC_IPADDR bcast;
    OFC_IPADDR mask;
    OFC_CONFIG_MODE netbios_mode;
    OFC_CONFIG_ICONFIG_TYPE itype;
    OFC_TCHAR *tstr;
    OFC_CCHAR *cstr;
    OFC_IPADDR *iparray;
    OFC_CONFIG *ofc_persist;
    PERSIST_REGISTER *subconfig;
    OFC_DOMNode *sub_node;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        error_state = OFC_FALSE;

        config_node = ofc_dom_get_element(config_dom, "of_core");

        if (config_node == OFC_NULL)
            error_state = OFC_TRUE;

        if (!error_state) {
            version = ofc_dom_get_attribute(config_node, "version");
            if (version != OFC_NULL) {
                if (ofc_strcmp(version, "0.0") != 0)
                    error_state = OFC_TRUE;
            } else
                error_state = OFC_TRUE;
        }

        if (!error_state) {
          log_node = ofc_dom_get_element(config_dom, "logging");
          if (log_node != OFC_NULL)
            {
              OFC_ULONG level;
              value = ofc_dom_get_element_cdata(log_node, "console");
              if (value != OFC_NULL)
                {
                  if (ofc_strcmp(value, "yes") == 0)
                    ofc_persist->log_console = OFC_TRUE;
                  else
                    ofc_persist->log_console = OFC_FALSE;
                }
              level = ofc_dom_get_element_cdata_ulong(log_node, "level");
              ofc_persist->log_level = (OFC_UINT) level;
            }
        }

        if (!error_state) {
            value = ofc_dom_get_element_cdata(config_dom, "devicename");
            if (value != OFC_NULL)
	      {
		ofc_log(OFC_LOG_INFO, "Device Name: %s\n", value);
                tstr = ofc_cstr2tstr(value);
                ofc_persist->workstation_name = tstr;
            }

            value = ofc_dom_get_element_cdata(config_dom, "uuid");
            if (value != OFC_NULL) {
                ofc_atouuid(value, ofc_persist->uuid);
            }

            value = ofc_dom_get_element_cdata(config_dom, "realm");
            if (value != OFC_NULL) {
              ofc_log(OFC_LOG_INFO, "Default Realm: %s\n", value);
              ofc_persist->default_realm = ofc_strdup(value);
            }

            value = ofc_dom_get_element_cdata(config_dom, "description");
            if (value != OFC_NULL) {
                tstr = ofc_cstr2tstr(value);
                ofc_persist->workstation_desc = tstr;
            }
            ip_node = ofc_dom_get_element(config_dom, "ip");
            if (ip_node != OFC_NULL) {
                value = ofc_dom_get_element_cdata(ip_node, "autoip");
                if (value != OFC_NULL) {
                    if (ofc_strcmp(value, "yes") == 0)
                        ofc_persist->enableAutoIP = OFC_TRUE;
                    else
                        ofc_persist->enableAutoIP = OFC_FALSE;
                }

                value = ofc_dom_get_element_cdata(ip_node, "netbios");
                if (value != OFC_NULL) {
                    if (ofc_strcmp(value, "yes") == 0)
                        ofc_persist->netbiosEnabled = OFC_TRUE;
                    else
                        ofc_persist->netbiosEnabled = OFC_FALSE;
                }

                interfaces_node =
                        ofc_dom_get_element(config_dom, "interfaces");
                if (interfaces_node != OFC_NULL) {
                    itype = OFC_CONFIG_ICONFIG_MANUAL;
                    value = ofc_dom_get_element_cdata(interfaces_node, "config");
                    if (value != OFC_NULL) {
                        if (ofc_strcmp(value, "auto") == 0)
                            itype = OFC_CONFIG_ICONFIG_AUTO;
                    }

                    ofc_persist_set_interface_type(itype);

                    if (ofc_persist->iconfig_type != OFC_CONFIG_ICONFIG_AUTO) {
                        interface_nodelist =
                                ofc_dom_get_elements_by_tag_name(interfaces_node,
                                                                 "interface");
                        if (interface_nodelist != OFC_NULL) {
                            for (i = 0; interface_nodelist[i] != OFC_NULL;
                                 i++);

                            ofc_persist_set_interface_count(i);

                            for (i = 0; i < ofc_persist_interface_count(); i++) {
                                value =
                                        ofc_dom_get_element_cdata(interface_nodelist[i],
                                                                  "ipaddress");
                                if (value != OFC_NULL)
                                    ofc_pton(value, &ipaddress);
                                else {
                                    ipaddress.ip_version = OFC_FAMILY_IP;
                                    ipaddress.u.ipv4.addr = OFC_INADDR_NONE;
                                }
                                value =
                                        ofc_dom_get_element_cdata(interface_nodelist[i],
                                                                  "bcast");
                                if (value != OFC_NULL)
                                    ofc_pton(value, &bcast);
                                else {
                                    bcast.ip_version = OFC_FAMILY_IP;
                                    bcast.u.ipv4.addr = OFC_INADDR_BROADCAST;
                                }
                                value =
                                        ofc_dom_get_element_cdata(interface_nodelist[i],
                                                                  "mask");
                                if (value != OFC_NULL)
                                    ofc_pton(value, &mask);
                                else {
                                    mask.ip_version = OFC_FAMILY_IP;
                                    mask.u.ipv4.addr = OFC_INADDR_ANY;
                                }

                                netbios_mode = OFC_DEFAULT_NETBIOS_MODE;
                                value =
                                        ofc_dom_get_element_cdata(interface_nodelist[i],
                                                                  "mode");
                                if (value != OFC_NULL) {
                                    for (j = 0;
                                         j < 4 &&
                                         ofc_strcmp(value,
                                                    strmode[j]) != 0;
                                         j++);
                                    if (j < 4)
                                        netbios_mode = j;
                                }

                                iparray = OFC_NULL;
                                num_wins = 0;
                                wins_node =
                                        ofc_dom_get_element(interface_nodelist[i],
                                                            "winslist");
                                if (wins_node != OFC_NULL) {
                                    wins_nodelist =
                                            ofc_dom_get_elements_by_tag_name(wins_node,
                                                                             "wins");
                                    if (wins_nodelist != OFC_NULL) {
                                        for (num_wins = 0;
                                             wins_nodelist[num_wins] !=
                                             OFC_NULL;
                                             num_wins++);
                                        iparray =
                                                ofc_malloc(sizeof(OFC_IPADDR) *
                                                           num_wins);
                                        for (j = 0; j < num_wins; j++) {
                                            value =
                                                    ofc_dom_get_cdata(wins_nodelist[j]);
                                            ofc_pton(value, &iparray[j]);
                                        }
                                        ofc_dom_destroy_node_list(wins_nodelist);
                                    }
                                }

                                value =
                                        ofc_dom_get_element_cdata(interface_nodelist[i],
                                                                  "master");

                                ofc_persist_set_interface_config(i, netbios_mode,
                                                                 &ipaddress,
                                                                 &bcast,
                                                                 &mask,
                                                                 value,
                                                                 num_wins,
                                                                 iparray);
                                ofc_free(iparray);
                            }
                            ofc_dom_destroy_node_list(interface_nodelist);
                        }
                    }
                }

                dns_node = ofc_dom_get_element(ip_node, "dnslist");
                if (dns_node != OFC_NULL) {
                    dns_nodelist =
                            ofc_dom_get_elements_by_tag_name(dns_node, "dns");
                    if (dns_nodelist != OFC_NULL) {
                        for (i = 0; dns_nodelist[i] != OFC_NULL; i++);
                        ofc_persist->netbios_dns.num_dns = i;
                        if (ofc_persist->netbios_dns.dns !=
                            OFC_NULL)
                            ofc_free(ofc_persist->netbios_dns.dns);
                        iparray = ofc_malloc(sizeof(OFC_IPADDR) * i);
                        ofc_persist->netbios_dns.dns = iparray;
                        for (i = 0; i < ofc_persist->netbios_dns.num_dns;
                             i++) {
                            value = ofc_dom_get_cdata(dns_nodelist[i]);
                            ofc_pton(value, &iparray[i]);
                        }
                        ofc_dom_destroy_node_list(dns_nodelist);
                    }
                }
            }

            drives_node = ofc_dom_get_element(config_dom, "drives");
            if (drives_node != OFC_NULL) {
                drives_nodelist =
                        ofc_dom_get_elements_by_tag_name(drives_node, "map");
                if (drives_nodelist != OFC_NULL) {
                    for (i = 0; drives_nodelist[i] != OFC_NULL; i++) {
                        OFC_LPCSTR lpBookmark;
                        OFC_LPCSTR lpDesc;
                        OFC_LPCSTR lpFile;
                        OFC_PATH *path;
                        OFC_BOOL thumbnail = OFC_FALSE;

                        map_node = drives_nodelist[i];
                        lpBookmark =
                                ofc_dom_get_element_cdata(map_node, "drive");
                        lpDesc =
                                ofc_dom_get_element_cdata(map_node, "description");

                        value =
                                ofc_dom_get_element_cdata(map_node, "thumbnail");
                        if (value != OFC_NULL) {
                            if (ofc_strcmp(value, "yes") == 0)
                                thumbnail = OFC_TRUE;
                            else
                                thumbnail = OFC_FALSE;
                        }

                        lpFile = ofc_dom_get_element_cdata(map_node, "path");
                        if (lpBookmark != OFC_NULL && lpFile != OFC_NULL) {
                            path = ofc_path_createA(lpFile);
                            if (ofc_path_remote(path))
			      {
				if (lpDesc == OFC_NULL)
				  lpDesc = "Remote Share";
				if (!ofc_path_add_mapA(lpBookmark, lpDesc,
                                                       path, OFC_FST_SMB,
                                                       thumbnail))
                                    ofc_path_delete(path);
                            } else {
				if (lpDesc == OFC_NULL)
				  lpDesc = "Local Bookmark";
                                if (!ofc_path_add_mapA(lpBookmark, lpDesc,
                                                       path, OFC_FST_BOOKMARKS,
                                                       thumbnail))
				  ofc_path_delete(path);
                            }
                        }
                    }
                    ofc_dom_destroy_node_list(drives_nodelist);
                }
            }

	    for (subconfig = ofc_queue_first(ofc_persist->subconfigs);
		 subconfig != OFC_NULL;
		 subconfig = ofc_queue_next(ofc_persist->subconfigs, subconfig))
	      {
		sub_node = ofc_dom_get_element(config_dom, subconfig->tag);
		if (sub_node != OFC_NULL)
		  {
		    (*subconfig->parse)(config_dom, sub_node);
		  }
	      }

            ofc_persist->loaded = OFC_TRUE;
        }
    }
}

OFC_BOOL
ofc_persist_register(OFC_CCHAR *tag,
		     OFC_VOID (*parse)(OFC_DOMNode *dom, OFC_DOMNode *sub_node),
		     OFC_BOOL (*make)(OFC_DOMNode *doc, OFC_DOMNode *sub_node)
		     )
{
  OFC_CONFIG *ofc_persist;
  PERSIST_REGISTER *subconfig;
  OFC_BOOL ret = OFC_FALSE;

  ofc_persist = ofc_get_config();
  if (ofc_persist != OFC_NULL)
    {
      ofc_lock(ofc_persist->config_lock);
      subconfig = ofc_malloc(sizeof (PERSIST_REGISTER));
      if (subconfig != OFC_NULL)
	{
	  subconfig->tag = ofc_strdup(tag);
	  subconfig->parse = parse;
	  subconfig->make = make;
	  ofc_enqueue (ofc_persist->subconfigs, subconfig);
	  ret = OFC_TRUE;
	}
      ofc_unlock(ofc_persist->config_lock);
    }
  return (ret);
}

OFC_BOOL
ofc_persist_unregister(OFC_CCHAR*tag)
{
  OFC_CONFIG *ofc_persist;
  PERSIST_REGISTER *subconfig;
  OFC_BOOL ret = OFC_FALSE;
  OFC_BOOL found;

  ofc_persist = ofc_get_config();
  if (ofc_persist != OFC_NULL)
    {
      ofc_lock(ofc_persist->config_lock);
      for (found = OFC_FALSE, subconfig = ofc_queue_first(ofc_persist->subconfigs);
	   subconfig != OFC_NULL && !found;)
	{
	  if (ofc_strcmp(subconfig->tag, tag) == 0)
	    found = OFC_TRUE;
	  else
	    subconfig = ofc_queue_next(ofc_persist->subconfigs, subconfig);
	}
      if (found)
	{
	  ofc_queue_unlink(ofc_persist->subconfigs, subconfig);
	  ofc_free(subconfig->tag);
	  ofc_free(subconfig);
	  ret = OFC_TRUE;
	}
      ofc_unlock(ofc_persist->config_lock);
    }
  return (ret);
}
#endif

static OFC_VOID
ofc_persist_lock(OFC_VOID) {
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL)
        ofc_lock(ofc_persist->config_lock);
}

static OFC_VOID
ofc_persist_unlock(OFC_VOID) {
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL)
        ofc_unlock(ofc_persist->config_lock);
}

#if defined(OFC_PERSIST)
OFC_CORE_LIB OFC_VOID
ofc_persist_print(OFC_LPVOID *buf, OFC_SIZET *len) {
    OFC_CONFIG *ofc_persist;
    OFC_DOMNode *config_dom;

    *len = 0;
    *buf = OFC_NULL;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();

        config_dom = ofc_persist_make_dom();

        if (config_dom != OFC_NULL) {
            *len = ofc_dom_sprint_document(OFC_NULL, 0, config_dom);
            *buf = (OFC_LPVOID) ofc_malloc(*len + 1);
            ofc_dom_sprint_document(*buf, *len + 1, config_dom);
            ofc_dom_destroy_document(config_dom);
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_save(OFC_LPCTSTR lpFileName) {
    OFC_LPVOID buf;
    OFC_HANDLE xml;
    OFC_DWORD dwLen;
    OFC_SIZET len;

    ofc_persist_print(&buf, &len);

    if (buf != OFC_NULL) {
        dwLen = (OFC_DWORD) len;
        xml = OfcCreateFileW(lpFileName,
                             OFC_GENERIC_WRITE,
                             OFC_FILE_SHARE_READ,
                             OFC_NULL,
                             OFC_CREATE_ALWAYS,
                             OFC_FILE_ATTRIBUTE_NORMAL,
                             OFC_HANDLE_NULL);

        if (xml != OFC_INVALID_HANDLE_VALUE) {
            OfcWriteFile(xml, buf, dwLen, &dwLen, OFC_HANDLE_NULL);
            OfcCloseHandle(xml);
        }

        ofc_free(buf);
    }
}

typedef struct {
    OFC_HANDLE handle;
} FILE_CONTEXT;

typedef struct {
  OFC_CHAR *buf;
  OFC_SIZET len;
} BUF_CONTEXT;

static OFC_SIZET
readFile(OFC_VOID *context, OFC_LPVOID buf, OFC_DWORD bufsize) {
    FILE_CONTEXT *fileContext;
    OFC_DWORD bytes_read;
    OFC_SIZET ret;

    fileContext = (FILE_CONTEXT *) context;

    if (!OfcReadFile(fileContext->handle, buf, bufsize,
                     &bytes_read, OFC_HANDLE_NULL))
        ret = -1;
    else
      {
        ret = bytes_read;
      }
    return (ret);
}

static OFC_SIZET
readBuf(OFC_VOID *context, OFC_LPVOID buf, OFC_DWORD bufsize) {
    BUF_CONTEXT *bufContext;
    OFC_DWORD bytes_read;
    OFC_SIZET ret;

    bufContext = (BUF_CONTEXT *) context;

    ret = OFC_MIN(bufsize, bufContext->len);
    ofc_memcpy(buf, bufContext->buf, ret);
    bufContext->len -= ret;
    bufContext->buf += ret;

    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_load(OFC_LPCTSTR lpFileName) {
    FILE_CONTEXT *fileContext;
    OFC_DOMNode *config_dom;

    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();

        fileContext = (FILE_CONTEXT *) ofc_malloc(sizeof(FILE_CONTEXT));

        fileContext->handle = OfcCreateFileW(lpFileName,
                                             OFC_GENERIC_READ,
                                             OFC_FILE_SHARE_READ,
                                             OFC_NULL,
                                             OFC_OPEN_EXISTING,
                                             OFC_FILE_ATTRIBUTE_NORMAL,
                                             OFC_HANDLE_NULL);

        config_dom = OFC_NULL;

        if (fileContext->handle != OFC_INVALID_HANDLE_VALUE &&
            fileContext->handle != OFC_HANDLE_NULL) {
            config_dom = ofc_dom_load_document(readFile,
                                               (OFC_VOID *) fileContext);

            OfcCloseHandle(fileContext->handle);
        } else
	  ofc_log(OFC_LOG_WARN, "Could not load config file %S\n", lpFileName);

        ofc_free(fileContext);

        if (config_dom != OFC_NULL) {
            ofc_persist_free();
            ofc_persist_parse_dom(config_dom);
            ofc_dom_destroy_document(config_dom);
        }

        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_loadbuf(OFC_LPVOID buf, OFC_SIZET len) {
    BUF_CONTEXT *bufContext;
    OFC_DOMNode *config_dom;

    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();

        config_dom = OFC_NULL;

        bufContext = (BUF_CONTEXT *) ofc_malloc(sizeof(BUF_CONTEXT));
        bufContext->buf = buf;
        bufContext->len = len;

        config_dom = ofc_dom_load_document(readBuf,
                                           (OFC_VOID *) bufContext);
        ofc_free(bufContext);

        if (config_dom != OFC_NULL) {
            ofc_persist_free();
            ofc_persist_parse_dom(config_dom);
            ofc_dom_destroy_document(config_dom);
        }

        ofc_persist_unlock();
    }
}

#endif

OFC_CORE_LIB OFC_VOID
ofc_persist_free(OFC_VOID) {
    OFC_INT i;
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        if (ofc_persist->default_realm != OFC_NULL) {
          ofc_free(ofc_persist->default_realm);
          ofc_persist->default_realm = OFC_NULL;
        }
        if (ofc_persist->workstation_name != OFC_NULL) {
            ofc_free(ofc_persist->workstation_name);
            ofc_persist->workstation_name = OFC_NULL;
        }
        if (ofc_persist->workstation_domain != OFC_NULL) {
            ofc_free(ofc_persist->workstation_domain);
            ofc_persist->workstation_domain = OFC_NULL;
        }
        if (ofc_persist->workstation_desc != OFC_NULL) {
            ofc_free(ofc_persist->workstation_desc);
            ofc_persist->workstation_desc = OFC_NULL;
        }
        if (ofc_persist->interface_config != OFC_NULL) {
            interface_config = ofc_persist->interface_config;

            for (i = 0; i < ofc_persist->interface_count; i++) {
                if (interface_config[i].winslist != OFC_NULL) {
                    ofc_free(interface_config[i].winslist);
                    interface_config[i].winslist = OFC_NULL;
                }
                interface_config[i].num_wins = 0;
                ofc_free(interface_config[i].master);
            }
            ofc_free(ofc_persist->interface_config);
            ofc_persist->interface_config = OFC_NULL;
            ofc_persist->interface_count = 0;
        }

        if (ofc_persist->netbios_dns.dns != OFC_NULL) {
            ofc_free(ofc_persist->netbios_dns.dns);
            ofc_persist->netbios_dns.num_dns = 0;
            ofc_persist->netbios_dns.dns = OFC_NULL;
        }

    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_default(OFC_VOID) {
    OFC_TCHAR *tstr;

    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();

        ofc_persist_free();

        ofc_persist->default_realm = OFC_NULL;
        tstr = ofc_cstr2tstr(OFC_DEFAULT_NAME);

        ofc_persist->workstation_name = tstr;

        tstr = ofc_cstr2tstr(OFC_DEFAULT_DESCR);
        ofc_persist->workstation_desc = tstr;

        ofc_persist->interface_count = 0;

        ofc_persist_set_interface_type(OFC_CONFIG_ICONFIG_AUTO);

        tstr = ofc_cstr2tstr(OFC_DEFAULT_DOMAIN);
        ofc_persist->workstation_domain = tstr;

        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_init(OFC_VOID) {
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();

    if (ofc_persist == OFC_NULL) {
        ofc_persist = ofc_malloc(sizeof(OFC_CONFIG));
        if (ofc_persist != OFC_NULL) {
            /*
             * Clear the configuration structure
             */
            ofc_memset(ofc_persist, '\0', sizeof(OFC_CONFIG));

            ofc_persist->update_count = 0;
            ofc_persist->config_lock = ofc_lock_init();
            ofc_persist->log_level = OFC_LOG_DEFAULT;
            ofc_persist->log_console = OFC_LOG_CONSOLE;
            ofc_persist->loaded = OFC_FALSE;
            ofc_persist->default_realm = OFC_NULL;
            ofc_persist->workstation_name = OFC_NULL;
            ofc_persist->workstation_domain = OFC_NULL;
            ofc_persist->workstation_desc = OFC_NULL;
            ofc_persist->interface_config = OFC_NULL;
            ofc_persist->netbios_dns.dns = OFC_NULL;
	    ofc_persist->subconfigs = ofc_queue_create();
            ofc_persist->netbiosEnabled = OFC_TRUE;

            ofc_set_config(ofc_persist);

            ofc_persist_default();
        }
    }
    event_queue = ofc_queue_create();
}

OFC_CORE_LIB OFC_VOID
ofc_persist_unload(OFC_VOID) {
    OFC_HANDLE hEvent;
    OFC_CONFIG *ofc_persist;
    PERSIST_REGISTER *subconfig;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        for (hEvent = (OFC_HANDLE) ofc_dequeue(event_queue);
             hEvent != OFC_HANDLE_NULL;
             hEvent = (OFC_HANDLE) ofc_dequeue(event_queue)) {
            ofc_event_destroy(hEvent);
        }
        ofc_queue_destroy(event_queue);

	for (subconfig = ofc_dequeue(ofc_persist->subconfigs);
	     subconfig != OFC_NULL;
	     subconfig = ofc_dequeue(ofc_persist->subconfigs))
	  {
	    ofc_free(subconfig->tag);
	    ofc_free(subconfig);
	  }
	ofc_queue_destroy(ofc_persist->subconfigs);

        ofc_persist_free();

        ofc_lock_destroy(ofc_persist->config_lock);

        ofc_free(ofc_persist);
        ofc_set_config(OFC_NULL);
    }
}

OFC_CORE_LIB OFC_BOOL
ofc_persist_loaded(OFC_VOID) {
    OFC_CONFIG *ofc_persist;
    OFC_BOOL ret;

    ret = OFC_FALSE;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ret = ofc_persist->loaded;
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_persistResetInterfaceConfig(OFC_VOID) {
    OFC_INT i;
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (ofc_persist->interface_config != OFC_NULL) {
            interface_config = ofc_persist->interface_config;
            for (i = 0; i < ofc_persist->interface_count; i++) {
                if (interface_config[i].winslist != OFC_NULL) {
                    ofc_free(interface_config[i].winslist);
                    interface_config[i].winslist = OFC_NULL;
                }
                interface_config[i].num_wins = 0;
                ofc_free(interface_config[i].master);
            }
            ofc_free(interface_config);
        }
        ofc_persist->interface_count = 0;
        ofc_persist->interface_config = OFC_NULL;
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_interface_type(OFC_CONFIG_ICONFIG_TYPE itype) {
    OFC_CONFIG_MODE netbios_mode;
    OFC_INT i;
    OFC_IPADDR ipaddress;
    OFC_IPADDR bcast;
    OFC_IPADDR mask;

    OFC_CONFIG *ofc_persist;
    OFC_INT num_wins;
    OFC_IPADDR *winslist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persistResetInterfaceConfig();

        ofc_persist->iconfig_type = itype;

        if (ofc_persist->iconfig_type == OFC_CONFIG_ICONFIG_AUTO) {
            ofc_persist_set_interface_count(ofc_net_interface_count());
            for (i = 0; i < ofc_persist->interface_count; i++) {
                netbios_mode = OFC_DEFAULT_NETBIOS_MODE;
                ofc_net_interface_addr(i, &ipaddress, &bcast, &mask);
                ofc_net_interface_wins(i, &num_wins, &winslist);
                ofc_persist_set_interface_config(i, netbios_mode,
                                                 &ipaddress, &bcast, &mask,
                                                 OFC_NULL,
                                                 num_wins, winslist);
                ofc_free(winslist);
            }
        }
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_netbios(OFC_BOOL enabled)
{
  OFC_CONFIG *ofc_persist;

  ofc_persist = ofc_get_config();
  if (ofc_persist != OFC_NULL)
    {
      ofc_persist->netbiosEnabled = enabled;
    }
}

OFC_CORE_LIB OFC_BOOL
ofc_persist_netbios(OFC_VOID)
{
  OFC_CONFIG *ofc_persist;
  OFC_BOOL enabled;

  ofc_persist = ofc_get_config();
  if (ofc_persist != OFC_NULL)
    {
      enabled = ofc_persist->netbiosEnabled;
    }
  return (enabled);
}

OFC_CORE_LIB OFC_CONFIG_ICONFIG_TYPE
ofc_persist_interface_config(OFC_VOID) {
    OFC_CONFIG *ofc_persist;
    OFC_CONFIG_ICONFIG_TYPE itype;

    itype = OFC_CONFIG_ICONFIG_AUTO;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL)
        itype = ofc_persist->iconfig_type;
    return (itype);
}

OFC_CORE_LIB OFC_INT
ofc_persist_wins_count(OFC_INT index) {
    OFC_INT ret;
    OFC_CONFIG *ofc_persist;
    OFC_CONFIG_ICONFIG *interface_config;

    ret = 0;
    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (index < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;
            ret = interface_config[index].num_wins;
        }
        ofc_persist_unlock();
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_wins_addr(OFC_INT xface, OFC_INT index, OFC_IPADDR *addr) {
    OFC_IPADDR *iparray;
    OFC_CONFIG *ofc_persist;
    OFC_CONFIG_ICONFIG *interface_config;

    addr->ip_version = OFC_FAMILY_IP;
    addr->u.ipv4.addr = OFC_INADDR_NONE;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (xface < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;

            if (index < interface_config[xface].num_wins) {
                iparray = interface_config[xface].winslist;
                ofc_memcpy(addr, &iparray[index], sizeof(OFC_IPADDR));
            }
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_interface_count(OFC_INT count) {
    OFC_INT i;
    OFC_CONFIG_ICONFIG *interface_config;

    OFC_CONFIG *ofc_persist;
    OFC_INT old_count;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        interface_config = ofc_persist->interface_config;

        old_count = ofc_persist->interface_count;
        ofc_persist->interface_count = count;

        for (i = count; i < old_count; i++) {
            if (interface_config[i].winslist != OFC_NULL)
                ofc_free(interface_config[i].winslist);
            interface_config[i].num_wins = 0;
            interface_config[i].winslist = OFC_NULL;
            if (interface_config[i].master != OFC_NULL)
                ofc_free(interface_config[i].master);
            interface_config[i].master = OFC_NULL;
        }

        interface_config =
                ofc_realloc(ofc_persist->interface_config,
                            sizeof(OFC_CONFIG_ICONFIG) * count);
        ofc_persist->interface_config = interface_config;

        for (i = old_count; i < count; i++) {
            interface_config[i].master = OFC_NULL;
            interface_config[i].winslist = OFC_NULL;
            interface_config[i].num_wins = 0;
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_remove_interface_config(OFC_IPADDR *ip) {
    OFC_INT i;
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;
    OFC_INT idx;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        interface_config = ofc_persist->interface_config;

        for (idx = 0; idx < ofc_persist->interface_count &&
                      !ofc_net_is_addr_equal(&interface_config[idx].ipaddress, ip);
             idx++);

        if (idx < ofc_persist->interface_count) {
            if (interface_config[idx].winslist != OFC_NULL)
                ofc_free(interface_config[idx].winslist);
            interface_config[idx].num_wins = 0;
            interface_config[idx].winslist = OFC_NULL;
            if (interface_config[idx].master != OFC_NULL)
                ofc_free(interface_config[idx].master);
            interface_config[idx].master = OFC_NULL;

            ofc_persist->interface_count--;

            for (i = idx; i < ofc_persist->interface_count; i++) {
                interface_config[i].netbios_mode =
                        interface_config[i + 1].netbios_mode;
                interface_config[i].ipaddress =
                        interface_config[i + 1].ipaddress;
                interface_config[i].bcast =
                        interface_config[i + 1].bcast;
                interface_config[i].mask =
                        interface_config[i + 1].mask;
                interface_config[i].num_wins =
                        interface_config[i + 1].num_wins;
                interface_config[i].winslist =
                        interface_config[i + 1].winslist;
                interface_config[i].master =
                        interface_config[i + 1].master;
            }

            interface_config =
                    ofc_realloc(ofc_persist->interface_config,
                                sizeof(OFC_CONFIG_ICONFIG) *
                                ofc_persist->interface_count);
            ofc_persist->interface_config = interface_config;
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_interface_config(OFC_INT i,
                                 OFC_CONFIG_MODE netbios_mode,
                                 OFC_IPADDR *ipaddress,
                                 OFC_IPADDR *bcast,
                                 OFC_IPADDR *mask,
                                 OFC_CHAR *master,
                                 OFC_INT num_wins,
                                 OFC_IPADDR *winslist) {
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CHAR *cstr;
    OFC_CONFIG *ofc_persist;
    OFC_INT j;
    OFC_IPADDR *iwinslist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (i < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;

            interface_config[i].netbios_mode = netbios_mode;
            if (ipaddress != OFC_NULL) {
                interface_config[i].ipaddress = *ipaddress;
            }
            if (bcast != OFC_NULL)
                interface_config[i].bcast = *bcast;
            if (mask != OFC_NULL)
                interface_config[i].mask = *mask;

            if (interface_config[i].winslist != OFC_NULL)
                ofc_free(interface_config[i].winslist);
            interface_config[i].num_wins = 0;
            interface_config[i].winslist = OFC_NULL;
            if (num_wins != 0 && winslist != OFC_NULL) {
                interface_config[i].num_wins = num_wins;
                iwinslist = ofc_malloc(sizeof(OFC_IPADDR) * num_wins);
                interface_config[i].winslist = iwinslist;
                for (j = 0; j < num_wins; j++)
                    iwinslist[j] = winslist[j];
            }

            if (master != OFC_NULL) {
                if (interface_config[i].master != OFC_NULL)
                    ofc_free(interface_config[i].master);

                cstr = ofc_strdup(master);
                interface_config[i].master = cstr;
            }
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_private(OFC_INT index, OFC_VOID *private) {
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (index < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;
            interface_config[index].private = private;
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_private(OFC_INT index, OFC_VOID **private) {
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;

    *private = OFC_NULL;
    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (index < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;
            *private = interface_config[index].private;
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_local_master(OFC_INT index, OFC_LPCSTR local_master) {
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CHAR *cstr;
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (index < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;

            if (interface_config[index].master != OFC_NULL)
                ofc_free(interface_config[index].master);
            interface_config[index].master = OFC_NULL;
            if (local_master != OFC_NULL) {
                cstr = ofc_strdup(local_master);
                interface_config[index].master = cstr;
            }
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_INT
ofc_persist_interface_count(OFC_VOID) {
    OFC_INT ret;
    OFC_CONFIG *ofc_persist;

    ret = 0;
    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ret = ofc_persist->interface_count;
    }
    return (ret);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_local_master(OFC_INT index, OFC_LPCSTR *local_master) {
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;

    *local_master = OFC_NULL;
    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (index < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;
            *local_master = interface_config[index].master;
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_interface_addr(OFC_INT index, OFC_IPADDR *addr,
                           OFC_IPADDR *pbcast, OFC_IPADDR *pmask) {
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;

    addr->ip_version = OFC_FAMILY_IP;
    addr->u.ipv4.addr = OFC_INADDR_NONE;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (index < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;

            if (addr != OFC_NULL)
                ofc_memcpy(addr, &interface_config[index].ipaddress,
                           sizeof(OFC_IPADDR));
            if (pbcast != OFC_NULL)
                ofc_memcpy(pbcast, &interface_config[index].bcast,
                           sizeof(OFC_IPADDR));
            if (pmask != OFC_NULL)
                ofc_memcpy(pmask, &interface_config[index].mask,
                           sizeof(OFC_IPADDR));
        }
        ofc_persist_unlock();
    }
}

OFC_CORE_LIB OFC_CONFIG_MODE
ofc_persist_interface_mode(OFC_INT index, OFC_INT *num_wins,
                           OFC_IPADDR **winslist) {
    OFC_CONFIG_MODE mode;
    OFC_CONFIG_ICONFIG *interface_config;
    OFC_CONFIG *ofc_persist;
    OFC_IPADDR *iwinslist;
    OFC_INT i;

    mode = OFC_DEFAULT_NETBIOS_MODE;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        if (index < ofc_persist->interface_count) {
            interface_config = ofc_persist->interface_config;
            mode = interface_config[index].netbios_mode;
            if (num_wins != OFC_NULL) {
                *num_wins = interface_config[index].num_wins;
                if (winslist != OFC_NULL) {
                    *winslist = OFC_NULL;
                    if (*num_wins > 0) {
                        *winslist =
                                ofc_malloc(sizeof(OFC_IPADDR) * *num_wins);
                        iwinslist = interface_config[index].winslist;
                        for (i = 0; i < *num_wins; i++) {
                            (*winslist)[i] = iwinslist[i];
                        }
                    }
                }
            }
        }
        ofc_persist_unlock();
    }

    return (mode);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_node_name(OFC_LPCTSTR *name,
                      OFC_LPCTSTR *workgroup,
                      OFC_LPCTSTR *desc) {
    OFC_CONFIG *ofc_persist;

    *name = OFC_NULL;
    *workgroup = OFC_NULL;
    *desc = OFC_NULL;
    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        *name = ofc_persist->workstation_name;
        *workgroup = ofc_persist->workstation_domain;
        *desc = ofc_persist->workstation_desc;
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_node_name(OFC_LPCTSTR name,
                          OFC_LPCTSTR workgroup,
                          OFC_LPCTSTR desc) {
    OFC_TCHAR *tstr;
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        if (ofc_persist->workstation_name != OFC_NULL)
            ofc_free(ofc_persist->workstation_name);
        tstr = ofc_tstrdup(name);
        ofc_persist->workstation_name = tstr;
        if (ofc_persist->workstation_domain != OFC_NULL)
            ofc_free(ofc_persist->workstation_domain);
        tstr = ofc_tstrdup(workgroup);
        ofc_persist->workstation_domain = tstr;
        if (ofc_persist->workstation_desc != OFC_NULL)
            ofc_free(ofc_persist->workstation_desc);
        tstr = ofc_tstrdup(desc);
        ofc_persist->workstation_desc = tstr;
    }
}

OFC_CORE_LIB OFC_VOID ofc_persist_set_logging(OFC_UINT log_level, OFC_BOOL log_console)
{
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL)
      {
        ofc_persist->log_level = log_level;
        ofc_persist->log_console = log_console;
      }
}

OFC_CORE_LIB OFC_UINT
ofc_persist_log_level(OFC_VOID)
{
    OFC_CONFIG *ofc_persist;
    OFC_UINT level;

    level = OFC_LOG_DEFAULT;
    
    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL)
      {
        level = ofc_persist->log_level;
      }
    return (level);
}
  
OFC_CORE_LIB OFC_BOOL
ofc_persist_log_console(OFC_VOID)
{
    OFC_CONFIG *ofc_persist;
    OFC_BOOL console;

    console = OFC_LOG_CONSOLE;
    
    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL)
      {
        console = ofc_persist->log_console;
      }
    return (console);
}
  
OFC_CORE_LIB OFC_VOID
ofc_persist_uuid(OFC_UUID *uuid) {
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_memcpy(uuid, ofc_persist->uuid, OFC_UUID_LEN);
    } else
        ofc_memset(uuid, '\0', OFC_UUID_LEN);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_uuid(OFC_UUID *uuid) {
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_memcpy(ofc_persist->uuid, uuid, OFC_UUID_LEN);
    }
}

OFC_CORE_LIB OFC_CCHAR *
ofc_persist_realm(OFC_VOID) {
    OFC_CONFIG *ofc_persist;
    OFC_CCHAR *realm;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
      realm = ofc_persist->default_realm;
    } else
      realm = OFC_NULL;
    return (realm);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_set_realm(OFC_CCHAR *realm) {
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
      if (ofc_persist->default_realm != OFC_NULL)
        {
          ofc_free(ofc_persist->default_realm);
          ofc_persist->default_realm = OFC_NULL;
        }
      ofc_persist->default_realm = ofc_strdup(realm);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_register_update(OFC_HANDLE hEvent) {
    ofc_enqueue(event_queue, (OFC_VOID *) hEvent);
    /*
     * Everyone who registers get's an initial event
     */
    ofc_event_set(hEvent);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_unregister_update(OFC_HANDLE hEvent) {
    ofc_queue_unlink(event_queue, (OFC_VOID *) hEvent);
}

OFC_CORE_LIB OFC_VOID
ofc_persist_notify(OFC_VOID) {
    OFC_HANDLE hEvent;

    for (hEvent = (OFC_HANDLE) ofc_queue_first(event_queue);
         hEvent != OFC_HANDLE_NULL;
         hEvent = (OFC_HANDLE) ofc_queue_next(event_queue, (OFC_VOID *) hEvent)) {
        ofc_event_set(hEvent);
    }
}

OFC_CORE_LIB OFC_VOID
ofc_persist_update(OFC_VOID) {
    OFC_CONFIG *ofc_persist;

    ofc_persist = ofc_get_config();
    if (ofc_persist != OFC_NULL) {
        ofc_persist_lock();
        ofc_persist->update_count++;
        ofc_persist_notify();
        ofc_persist_unlock();
    }
}

