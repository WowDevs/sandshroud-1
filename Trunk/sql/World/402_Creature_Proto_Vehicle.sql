ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `vehiclecreatureid` `vehiclecreatureid` MEDIUMINT(8) NOT NULL DEFAULT '0' FIRST;
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `increasehealthbydriver` `increasehealthbydriver` TINYINT(3) NOT NULL DEFAULT '0' AFTER `vehiclecreatureid`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `healthforitemlevel` `healthforitemlevel` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `increasehealthbydriver`; 
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `spell1` `spell1` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `healthforitemlevel`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `spell2` `spell2` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `spell1`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `spell3` `spell3` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `spell2`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `spell4` `spell4` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `spell3`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `spell5` `spell5` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `spell4`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `spell6` `spell6` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `spell5`; 
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat1` `accessoryseat0` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `spell6`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat2` `accessoryseat1` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `accessoryseat0`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat3` `accessoryseat2` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `accessoryseat1`;  
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat4` `accessoryseat3` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `accessoryseat2`;
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat5` `accessoryseat4` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `accessoryseat3`;
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat6` `accessoryseat5` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `accessoryseat4`;
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat7` `accessoryseat6` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `accessoryseat5`;
ALTER TABLE `creature_proto_vehicle` CHANGE COLUMN `accessoryseat8` `accessoryseat7` MEDIUMINT(8) NOT NULL DEFAULT '0' AFTER `accessoryseat6`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath0` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat0`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable0` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath0`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath1` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat1`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable1` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath1`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath2` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat2`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable2` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath2`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath3` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat3`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable3` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath3`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath4` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat4`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable4` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath4`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath5` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat5`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable5` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath5`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath6` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat6`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable6` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath6`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `ejectondeath7` INT(10) NOT NULL DEFAULT '0' AFTER `accessoryseat7`;
ALTER TABLE `creature_proto_vehicle` ADD COLUMN `unselectable7` INT(10) NOT NULL DEFAULT '0' AFTER `ejectondeath7`;