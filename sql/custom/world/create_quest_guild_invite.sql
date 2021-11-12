DROP TABLE IF EXISTS `quest_guild_invite`;

CREATE TABLE `quest_guild_invite` (
  `quest_id` integer unsigned NOT NULL,
  `guild_id` integer unsigned NOT NULL,
  `rank_id`  integer unsigned NOT NULL,
  PRIMARY KEY (`quest_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
