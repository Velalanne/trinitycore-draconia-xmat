DROP TABLE IF EXISTS `dnd_class`;

CREATE TABLE `dnd_class` (
    `id`           integer unsigned NOT NULL,
    `description`  VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;