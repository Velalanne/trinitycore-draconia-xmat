-- 
DELETE FROM `creature` WHERE `guid` IN (78359,78360,78361,78362,78363,78364,78365,78366,78320,78321,78322);
DELETE FROM `creature_addon` WHERE `guid` IN (78359,78360,78361,78362,78363,78364,78365,78366);
DELETE FROM `spawn_group` WHERE `spawnType`=0 AND `spawnId` IN (78359,78360,78361,78362,78363,78364,78365,78366);
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`) VALUES
(78359, 22291, 530, 1, 1, 0, 0, 2774.522, 7020.976, 370.8136, 2.12930200, 300, 0, 0, 7380, 0, 0),
(78360, 22291, 530, 1, 1, 0, 0, 2818.670, 7049.295, 370.3463, 1.95476900, 300, 0, 0, 7380, 0, 0),
(78361, 22291, 530, 1, 1, 0, 0, 2824.512, 7017.276, 370.0880, 5.74213300, 300, 0, 0, 7380, 0, 0),
(78362, 22291, 530, 1, 1, 0, 0, 2931.092, 7026.913, 367.9164, 3.64773800, 300, 0, 0, 7380, 0, 0),
(78363, 22291, 530, 1, 1, 0, 0, 2981.310, 6879.979, 370.3918, 1.53589000, 300, 0, 0, 7380, 0, 0),
(78364, 22291, 530, 1, 1, 0, 0, 2944.612, 6819.599, 366.6967, 3.80481800, 300, 0, 0, 7380, 0, 0),
(78365, 22291, 530, 1, 1, 0, 0, 2953.470, 6870.954, 370.7219, 3.10668600, 300, 0, 0, 7380, 0, 0),
(78366, 22291, 530, 1, 1, 0, 0, 3016.538, 6863.845, 370.0450, 0.05235988, 300, 0, 0, 7380, 0, 0),
(78320, 22291, 530, 1, 1, 0, 0, 3023.406, 6810.775, 374.3828, 5.96902600, 300, 0, 0, 7380, 0, 0),
(78321, 22291, 530, 1, 1, 0, 0, 2992.254, 7045.759, 369.6008, 5.34070700, 300, 0, 0, 7380, 0, 0),
(78322, 22291, 530, 1, 1, 0, 0, 2938.332, 7097.362, 370.2270, 2.49582100, 300, 0, 0, 7380, 0, 0);
