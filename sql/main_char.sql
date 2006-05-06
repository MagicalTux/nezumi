# -------------------------------------------------------

# Database: Ragnarok
# Table: 'cart_inventory'
# 
CREATE TABLE IF NOT EXISTS `cart_inventory` (
  `id` int(11) NOT NULL auto_increment,
  `char_id` int(11) NOT NULL default '0',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '0',
  `equip` mediumint(8) unsigned NOT NULL default '0',
  `identify` smallint(6) NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `broken` int(11) NOT NULL default '0',
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'char'
# 
CREATE TABLE IF NOT EXISTS `char` (
  `char_id` int(11) NOT NULL auto_increment,
  `account_id` int(11) NOT NULL default '0',
  `char_num` tinyint(4) NOT NULL default '0',
  `name` varchar(30) NOT NULL default '',
  `class` int(11) NOT NULL default '0',
  `base_level` bigint(20) unsigned NOT NULL default '1',
  `job_level` bigint(20) unsigned NOT NULL default '1',
  `base_exp` bigint(20) NOT NULL default '0',
  `job_exp` bigint(20) NOT NULL default '0',
  `zeny` int(11) NOT NULL default '500',
  `str` int(11) unsigned NOT NULL default '0',
  `agi` int(11) unsigned NOT NULL default '0',
  `vit` int(11) unsigned NOT NULL default '0',
  `int` int(11) unsigned NOT NULL default '0',
  `dex` int(11) unsigned NOT NULL default '0',
  `luk` int(11) unsigned NOT NULL default '0',
  `max_hp` int(11) NOT NULL default '0',
  `hp` int(11) NOT NULL default '0',
  `max_sp` int(11) NOT NULL default '0',
  `sp` int(11) NOT NULL default '0',
  `status_point` int(11) NOT NULL default '0',
  `skill_point` int(11) NOT NULL default '0',
  `option` int(11) NOT NULL default '0',
  `karma` int(11) NOT NULL default '0',
  `manner` int(11) NOT NULL default '0',
  `party_id` int(11) NOT NULL default '0',
  `guild_id` int(11) NOT NULL default '0',
  `pet_id` int(11) NOT NULL default '0',
  `hair` tinyint(4) NOT NULL default '0',
  `hair_color` int(11) NOT NULL default '0',
  `clothes_color` tinyint(4) NOT NULL default '0',
  `weapon` int(11) NOT NULL default '1',
  `shield` int(11) NOT NULL default '0',
  `head_top` int(11) NOT NULL default '0',
  `head_mid` int(11) NOT NULL default '0',
  `head_bottom` int(11) NOT NULL default '0',
  `last_map` varchar(20) NOT NULL default 'new_5-1.gat',
  `last_x` int(4) NOT NULL default '53',
  `last_y` int(11) NOT NULL default '111',
  `save_map` varchar(20) NOT NULL default 'new_5-1.gat',
  `save_x` int(11) NOT NULL default '53',
  `save_y` int(11) NOT NULL default '111',
  `partner_id` int(11) NOT NULL default '0',
  `online` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`char_id`),
  KEY `account_id` (`account_id`),
  KEY `party_id` (`party_id`),
  KEY `guild_id` (`guild_id`),
  KEY `online` (`online`)
) TYPE=MyISAM AUTO_INCREMENT=500000;
# before: 150000, now 500000 because floor items have id from 0 to 499999

# Database: Ragnarok
# Table: 'charlog'
# 
CREATE TABLE IF NOT EXISTS `charlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_msg` varchar(255) NOT NULL default 'char select',
  `account_id` int(11) NOT NULL default '0',
  `char_num` tinyint(4) NOT NULL default '0',
  `name` varchar(30) NOT NULL default '',
  `str` int(11) unsigned NOT NULL default '0',
  `agi` int(11) unsigned NOT NULL default '0',
  `vit` int(11) unsigned NOT NULL default '0',
  `int` int(11) unsigned NOT NULL default '0',
  `dex` int(11) unsigned NOT NULL default '0',
  `luk` int(11) unsigned NOT NULL default '0',
  `hair` tinyint(4) NOT NULL default '0',
  `hair_color` int(11) NOT NULL default '0'
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'global_reg_value'
# 
CREATE TABLE IF NOT EXISTS `global_reg_value` (
  `char_id` int(11) NOT NULL default '0',
  `str` varchar(32) NOT NULL default '',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY (`char_id`, `str`)
) TYPE=MyISAM;

# Database: Ragnarok
# Table: 'account_reg_db'
# 
CREATE TABLE IF NOT EXISTS `account_reg_db` (
  `account_id` int(11) NOT NULL default '0',
  `str` varchar(32) NOT NULL default '',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY (`account_id`, `str`)
) TYPE = MyISAM;

# Database: Ragnarok
# Table: 'guild'
# 
CREATE TABLE IF NOT EXISTS `guild` (
  `guild_id` int(11) NOT NULL default '10000',
  `name` varchar(24) NOT NULL default '',
  `master` varchar(24) NOT NULL default '',
  `guild_lv` smallint(6) NOT NULL default '0',
  `connect_member` smallint(6) NOT NULL default '0',
  `max_member` smallint(6) NOT NULL default '0',
  `average_lv` smallint(6) NOT NULL default '0',
  `exp` int(11) NOT NULL default '0',
  `next_exp` int(11) NOT NULL default '0',
  `skill_point` int(11) NOT NULL default '0',
  `castle_id` int(11) NOT NULL default '-1',
  `mes1` varchar(60) NOT NULL default '',
  `mes2` varchar(120) NOT NULL default '',
  `emblem_len` int(11) NOT NULL default '0',
  `emblem_id` int(11) NOT NULL default '0',
  `emblem_data` blob NOT NULL,
  PRIMARY KEY (`guild_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'guild_alliance'
# 
CREATE TABLE IF NOT EXISTS `guild_alliance` (
  `guild_id` int(11) NOT NULL default '0',
  `opposition` int(11) NOT NULL default '0',
  `alliance_id` int(11) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'guild_castle'
# 
CREATE TABLE IF NOT EXISTS `guild_castle` (
  `castle_id` int(11) NOT NULL default '0',
  `guild_id` int(11) NOT NULL default '0',
  `economy` int(11) NOT NULL default '0',
  `defense` int(11) NOT NULL default '0',
  `triggerE` int(11) NOT NULL default '0',
  `triggerD` int(11) NOT NULL default '0',
  `nextTime` int(11) NOT NULL default '0',
  `payTime` int(11) NOT NULL default '0',
  `createTime` int(11) NOT NULL default '0',
  `visibleC` int(11) NOT NULL default '0',
  `visibleG0` int(11) NOT NULL default '0',
  `visibleG1` int(11) NOT NULL default '0',
  `visibleG2` int(11) NOT NULL default '0',
  `visibleG3` int(11) NOT NULL default '0',
  `visibleG4` int(11) NOT NULL default '0',
  `visibleG5` int(11) NOT NULL default '0',
  `visibleG6` int(11) NOT NULL default '0',
  `visibleG7` int(11) NOT NULL default '0',
  `gHP0` int(11) NOT NULL default '0',
  `ghP1` int(11) NOT NULL default '0',
  `gHP2` int(11) NOT NULL default '0',
  `gHP3` int(11) NOT NULL default '0',
  `gHP4` int(11) NOT NULL default '0',
  `gHP5` int(11) NOT NULL default '0',
  `gHP6` int(11) NOT NULL default '0',
  `gHP7` int(11) NOT NULL default '0',
  PRIMARY KEY (`castle_id`),
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'guild_expulsion'
# 
CREATE TABLE IF NOT EXISTS `guild_expulsion` (
  `guild_id` int(11) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `mes` varchar(40) NOT NULL default '',
  `acc` varchar(40) NOT NULL default '',
  `account_id` int(11) NOT NULL default '0',
  `rsv1` int(11) NOT NULL default '0',
  `rsv2` int(11) NOT NULL default '0',
  `rsv3` int(11) NOT NULL default '0',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'guild_member'
# 
CREATE TABLE IF NOT EXISTS `guild_member` (
  `guild_id` int(11) NOT NULL default '0',
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `hair` smallint(6) NOT NULL default '0',
  `hair_color` smallint(6) NOT NULL default '0',
  `gender` smallint(6) NOT NULL default '0',
  `class` smallint(6) NOT NULL default '0',
  `lv` smallint(6) NOT NULL default '0',
  `exp` bigint(20) NOT NULL default '0',
  `exp_payper` int(11) NOT NULL default '0',
  `online` tinyint(4) NOT NULL default '0',
  `position` smallint(6) NOT NULL default '0',
  `rsv1` int(11) NOT NULL default '0',
  `rsv2` int(11) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  KEY `guild_id` (`guild_id`),
  KEY `account_id` (`account_id`),
  KEY `char_id` (`char_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'guild_position'
# 
CREATE TABLE IF NOT EXISTS `guild_position` (
  `guild_id` int(11) NOT NULL default '0',
  `position` smallint(6) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `mode` int(11) NOT NULL default '0',
  `exp_mode` int(11) NOT NULL default '0',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'guild_skill'
# 
CREATE TABLE IF NOT EXISTS `guild_skill` (
  `guild_id` int(11) NOT NULL default '0',
  `id` int(11) NOT NULL default '0',
  `lv` int(11) NOT NULL default '0',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'guild_storage'
# 
CREATE TABLE IF NOT EXISTS `guild_storage` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `guild_id` int(11) NOT NULL default '0',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '0',
  `equip` mediumint(8) unsigned NOT NULL default '0',
  `identify` smallint(6) NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `broken` int(11) NOT NULL default '0',
  PRIMARY KEY (`id`),
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'interlog'
# 
CREATE TABLE IF NOT EXISTS `interlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `log` varchar(255) NOT NULL default ''
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'inventory'
# 
CREATE TABLE IF NOT EXISTS `inventory` (
  `id` int(11) NOT NULL auto_increment,
  `char_id` int(11) NOT NULL default '0',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '0',
  `equip` mediumint(8) unsigned NOT NULL default '0',
  `identify` smallint(6) NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `broken` int(11) NOT NULL default '0',
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'memo'
# 
CREATE TABLE IF NOT EXISTS `memo` (
  `memo_id` int(11) NOT NULL auto_increment,
  `char_id` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default '',
  `x` mediumint(9) NOT NULL default '0',
  `y` mediumint(9) NOT NULL default '0',
  PRIMARY KEY  (`memo_id`),
  KEY `char_id` (`char_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'party'
# 
CREATE TABLE IF NOT EXISTS `party` (
  `party_id` int(11) NOT NULL default '100',
  `name` char(100) NOT NULL default '',
  `exp` int(11) NOT NULL default '0',
  `item` int(11) NOT NULL default '0',
  `leader_id` int(11) NOT NULL default '0',
  PRIMARY KEY (`party_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'pet'
# 
CREATE TABLE IF NOT EXISTS `pet` (
  `pet_id` int(11) NOT NULL auto_increment,
  `class` mediumint(9) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `level` tinyint(4) NOT NULL default '0',
  `egg_id` int(11) NOT NULL default '0',
  `equip` mediumint(8) unsigned NOT NULL default '0',
  `intimate` mediumint(9) NOT NULL default '0',
  `hungry` mediumint(9) NOT NULL default '0',
  `rename_flag` tinyint(4) NOT NULL default '0',
  `incuvate` int(11) NOT NULL default '0',
  PRIMARY KEY (`pet_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'ragsrvinfo'
# 
CREATE TABLE IF NOT EXISTS `ragsrvinfo` (
  `index` int(11) NOT NULL default '0',
  `name` varchar(40) NOT NULL default '',
  `exp` int(11) NOT NULL default '0',
  `jexp` int(11) NOT NULL default '0',
  `drop` int(11) NOT NULL default '0',
  `motd` varchar(255) NOT NULL default ''
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'skill'
# 
CREATE TABLE IF NOT EXISTS `skill` (
  `char_id` int(11) NOT NULL default '0',
  `id` int(11) NOT NULL default '0',
  `lv` tinyint(4) NOT NULL default '0',
  PRIMARY KEY (`char_id`,`id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'sc_data'
# 
CREATE TABLE IF NOT EXISTS `sc_data` (
  `char_id` int(11) unsigned NOT NULL,
  `type` smallint(11) unsigned NOT NULL,
  `tick` int(11) NOT NULL,
  `val1` int(11) NOT NULL default '0',
  `val2` int(11) NOT NULL default '0',
  `val3` int(11) NOT NULL default '0',
  `val4` int(11) NOT NULL default '0',
  KEY (`char_id`)
) TYPE=MyISAM;

# Database: Ragnarok
# Table: 'storage'
# 
CREATE TABLE IF NOT EXISTS `storage` (
  `id` int(11) NOT NULL auto_increment,
  `account_id` int(11) NOT NULL default '0',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '0',
  `equip` mediumint(8) unsigned NOT NULL default '0',
  `identify` smallint(6) NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `broken` int(11) NOT NULL default '0',
  PRIMARY KEY (`id`),
  KEY `account_id` (`account_id`)
) TYPE=MyISAM; 

# Database: Ragnarok
# Table: 'friends'
# 
CREATE TABLE IF NOT EXISTS `friends` (
  `char_id` int(11) NOT NULL default '0',
  `friend_id` int(11) NOT NULL default '0',
  PRIMARY KEY (`char_id`,`friend_id`)
) TYPE=MyISAM;

