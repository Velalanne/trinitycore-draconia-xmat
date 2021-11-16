DROP TABLE IF EXISTS `dnd_race`;

CREATE TABLE `dnd_race` (
    `id`                integer unsigned NOT NULL,
    `base_strength`     smallint unsigned NOT NULL,
    `base_dexterity`    smallint unsigned NOT NULL,
    `base_constitution` smallint unsigned NOT NULL,
    `base_intelligence` smallint unsigned NOT NULL,
    `base_wisdom`       smallint unsigned NOT NULL,
    `base_charisma`     smallint unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;