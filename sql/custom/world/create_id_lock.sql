DROP TABLE IF EXISTS `id_lock`;

CREATE TABLE `id_lock` (
  `guid` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `gob_entry` mediumint(8) unsigned NOT NULL,
  `char_entry` mediumint(8) unsigned NOT NULL,
  `lock_id` smallint(5) unsigned NOT NULL,
  `x` FLOAT NOT NULL DEFAULT '0',
  `y` FLOAT NOT NULL DEFAULT '0',
  `z` FLOAT NOT NULL DEFAULT '0',
  `orientation` FLOAT NOT NULL DEFAULT '0',
  `map_id` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `price` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `duration_open` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `duration_owner` mediumint(8) unsigned NOT NULL DEFAULT 0,
  `obtained` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
