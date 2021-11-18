DROP TABLE IF EXISTS `managed_guild`;

CREATE TABLE `managed_guild` (
  `guild_id` integer unsigned NOT NULL,
  `invite_quest_id` integer unsigned NOT NULL,
  `leave_quest_id`  integer unsigned NOT NULL,
  `rank_id`  integer unsigned NOT NULL,
  `faction_id`  integer unsigned NOT NULL,
  `leave_reputation_value`  integer NOT NULL,
  `key_item_entry`  integer unsigned NOT NULL,
  PRIMARY KEY (`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
