DROP TABLE IF EXISTS `dnd_proficiency`;

CREATE TABLE `dnd_proficiency` (
    `id`                integer unsigned NOT NULL,
	`class_id`          integer unsigned NOT NULL,
    `dnd_level`         smallint unsigned NOT NULL,
    `prof_strength`     smallint unsigned NOT NULL,
    `prof_dexterity`    smallint unsigned NOT NULL,
    `prof_constitution` smallint unsigned NOT NULL,
    `prof_intelligence` smallint unsigned NOT NULL,
    `prof_wisdom`       smallint unsigned NOT NULL,
    `prof_charisma`     smallint unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;