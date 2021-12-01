DROP TABLE IF EXISTS `dnd_character`;

CREATE TABLE `dnd_character` (
    `id`         integer unsigned NOT NULL comment 'character.guid',
    `class_id` integer unsigned NOT NULL,
    `race_id`  integer unsigned NOT NULL,
	`dnd_level`    integer unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;
