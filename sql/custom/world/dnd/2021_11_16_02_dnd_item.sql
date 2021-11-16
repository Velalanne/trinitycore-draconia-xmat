DROP TABLE IF EXISTS `dnd_item`;

CREATE TABLE `dnd_item` (
    `id`           integer unsigned NOT NULL comment 'references item_template.entry',
    `melee_hit`    smallint unsigned default 0 NOT NULL,
    `ranged_hit`   smallint unsigned default 0 NOT NULL,
    `spell_hit`    smallint unsigned default 0 NOT NULL,
    `strength`     smallint unsigned default 0 NOT NULL,
    `dexterity`    smallint unsigned default 0 NOT NULL,
    `constitution` smallint unsigned default 0 NOT NULL,
    `intelligence` smallint unsigned default 0 NOT NULL,
    `wisdom`       smallint unsigned default 0 NOT NULL,
    `charisma`     smallint unsigned default 0 NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;