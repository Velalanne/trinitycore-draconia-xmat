DROP TABLE IF EXISTS `character_modification`;

CREATE TABLE `character_modification` (
  `guid` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `scale` FLOAT NOT NULL DEFAULT '0',
  `display_id` mediumint(8) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
