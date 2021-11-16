DROP TABLE IF EXISTS `dnd_character`;

CREATE TABLE `dnd_character` (
    `guid`         integer unsigned NOT NULL comment 'character.guid',
    `strength`     smallint unsigned NOT NULL,
    `dexterity`    smallint unsigned NOT NULL,
    `constitution` smallint unsigned NOT NULL,
    `intelligence` smallint unsigned NOT NULL,
    `wisdom`       smallint unsigned NOT NULL,
    `charisma`     smallint unsigned NOT NULL,
    `dnd_class_id` integer unsigned NOT NULL,
    `dnd_race_id`  integer unsigned NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;
