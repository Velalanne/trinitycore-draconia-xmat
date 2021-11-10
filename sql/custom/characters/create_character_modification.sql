DROP TABLE IF EXISTS `character_modification`;

CREATE TABLE `character_modification`
(
	`guid` INT unsigned,
	`scale` FLOAT DEFAULT 1,
	`display_id` INT unsigned DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;