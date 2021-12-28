/*
 * Wazuh-DB settings manager
 * Copyright (C) 2015-2022, Wazuh Inc.
 * Dec 17, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "shared.h"
#include "config.h"
#include "config/wazuh_db-config.h"
#include "wazuh_db/wdb.h"
#include "headers/string_op.h"

int Read_WazuhDB(const OS_XML *xml, XML_NODE chld_node) {
    const char* xml_backup = "backup";
    const char* xml_database = "database";
    const char* xml_database_global = "global";

    for(int i = 0; chld_node[i]; i++) {
        if (!chld_node[i]->element) {
            merror(XML_ELEMNULL);
            return OS_INVALID;
        } else if (!strcmp(chld_node[i]->element, xml_backup)) {
            if(chld_node[i]->attributes && chld_node[i]->attributes[0] && !strcmp(chld_node[i]->attributes[0], xml_database)) {
                if(chld_node[i]->values && chld_node[i]->values[0] && !strcmp(chld_node[i]->values[0], xml_database_global)) {
                    return Read_WazuhDB_Backup(xml, chld_node[i], WDB_GLOBAL_BACKUP);
                } else {
                    merror(XML_VALUEERR, chld_node[i]->attributes[0], chld_node[i]->values && chld_node[i]->values[0] ? chld_node[i]->values[0] : "");
                    return OS_INVALID;
                }
            } else {
                merror(XML_INVATTR, chld_node[i]->attributes && chld_node[i]->attributes[0] ? chld_node[i]->attributes[0] : "", chld_node[i]->element);
                return OS_INVALID;
            }
        } else {
            merror(XML_INVELEM, chld_node[i]->element);
            return OS_INVALID;
        }
    }

    return OS_SUCCESS;
}

int Read_WazuhDB_Backup(const OS_XML *xml, xml_node * node, int const BACKUP_NODE) {
    const char* xml_enabled = "enabled";
    const char* xml_interval = "interval";
    const char* xml_max_files = "max_files";
    XML_NODE chld_node = NULL;

    chld_node = OS_GetElementsbyNode(xml, node);
    if(!chld_node) {
        merror(XML_ELEMNULL);
        return OS_INVALID;
    }

    for (int i = 0; chld_node[i]; i++) {
        if (!chld_node[i]->element) {
            merror(XML_ELEMNULL);
            os_free(chld_node);
            return OS_INVALID;
        } else if (!chld_node[i]->content) {
            merror(XML_VALUENULL, chld_node[i]->element);
            os_free(chld_node);
            return OS_INVALID;
        } else if (!strcmp(chld_node[i]->element, xml_enabled)) {
            short tmp_bool = eval_bool(chld_node[i]->content);

            if (tmp_bool < 0) {
                merror(XML_VALUEERR, chld_node[i]->element, chld_node[i]->content);
                os_free(chld_node);
                return OS_INVALID;
            }

            wconfig.wdb_backup_settings[BACKUP_NODE]->enabled = tmp_bool;
        } else if (!strcmp(chld_node[i]->element, xml_interval)) {
            long time_value = w_parse_time(chld_node[i]->content);

            if (time_value > 0) {
                wconfig.wdb_backup_settings[BACKUP_NODE]->interval = time_value;
            } else {
                merror(XML_VALUEERR, chld_node[i]->element, chld_node[i]->content);
                os_free(chld_node);
                return OS_INVALID;
            }
        } else if (!strcmp(chld_node[i]->element, xml_max_files)) {
            if (!OS_StrIsNum(chld_node[i]->content)) {
                merror(XML_VALUEERR, chld_node[i]->element, chld_node[i]->content);
                os_free(chld_node);
                return (OS_INVALID);
            }

            wconfig.wdb_backup_settings[BACKUP_NODE]->max_files = atoi(chld_node[i]->content);

            if (wconfig.wdb_backup_settings[BACKUP_NODE]->max_files <= 0) {
                merror(XML_VALUEERR, chld_node[i]->element, chld_node[i]->content);
                os_free(chld_node);
                return (OS_INVALID);
            }
        } else {
            merror(XML_INVELEM, chld_node[i]->element);
            os_free(chld_node);
            return OS_INVALID;
        }
    }

    os_free(chld_node);
    return OS_SUCCESS;
}

void wdb_init_conf() {
    os_calloc(WDB_LAST_BACKUP, sizeof(wdb_backup_settings_node*), wconfig.wdb_backup_settings);

    for (int i = 0; i < WDB_LAST_BACKUP; i++) {
        os_calloc(1, sizeof(wdb_backup_settings_node*), wconfig.wdb_backup_settings[i]);
        wconfig.wdb_backup_settings[i]->enabled = true;
        wconfig.wdb_backup_settings[i]->interval = 86400;
        wconfig.wdb_backup_settings[i]->max_files = 3;
    }
}

void wdb_free_conf() {
    for (int i = 0; i < WDB_LAST_BACKUP; i++) {
        os_free(wconfig.wdb_backup_settings[i]);
    }
    os_free(wconfig.wdb_backup_settings);
}