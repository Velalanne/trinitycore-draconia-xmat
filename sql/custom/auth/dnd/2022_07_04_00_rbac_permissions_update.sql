INSERT IGNORE INTO rbac_permissions(id, name) VALUES(889, 'dnd oc');
INSERT IGNORE INTO rbac_permissions(id, name) VALUES(890, 'dnd oc target');

INSERT IGNORE INTO rbac_linked_permissions(id, linkedId) VALUES(195, 889);
INSERT IGNORE INTO rbac_linked_permissions(id, linkedId) VALUES(193, 890);