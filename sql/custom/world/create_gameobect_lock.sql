DROP TABLE IF EXISTS `gameobject_lock`;

CREATE TABLE `gameobject_lock` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `gob_guid` mediumint(8) unsigned NOT NULL,
  `char_entry` mediumint(8) unsigned NOT NULL,
  `lock_id` smallint(5) unsigned NOT NULL,
  `x` FLOAT NOT NULL DEFAULT '0',
  `y` FLOAT NOT NULL DEFAULT '0',
  `z` FLOAT NOT NULL DEFAULT '0',
  `orientation` FLOAT NOT NULL DEFAULT '0',
  `map_id` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `duration_open` mediumint(8) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
