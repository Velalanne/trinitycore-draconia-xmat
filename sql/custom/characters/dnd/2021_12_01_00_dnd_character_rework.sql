DROP TABLE IF EXISTS `dnd_character`;

CREATE TABLE `dnd_character` (
    `guid`         integer unsigned NOT NULL comment 'character.guid',
    `dclass_id` integer unsigned NOT NULL,
    `race_id`  integer unsigned NOT NULL,
	`dnd_level`    integer unsigned NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;
