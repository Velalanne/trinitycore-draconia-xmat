DROP TABLE IF EXISTS `dnd_armor_class`;

CREATE TABLE `dnd_armor_class` (
    `type`   smallint unsigned NOT NULL,
    `rarity` smallint unsigned NOT NULL,
    `value`  smallint unsigned NOT NULL,
  PRIMARY KEY (`type`, `rarity`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;