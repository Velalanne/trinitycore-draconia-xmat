-- Enraged Foulweald shouldn't have loot table
DELETE FROM `creature_loot_template` WHERE `Entry` = 12921;
UPDATE `creature_template` SET `lootid` = 0 WHERE `entry` = 12921;

DELETE FROM `creature_loot_template` WHERE `Chance` < 0.11 AND `Item` IN (765,785,2447,2449,2450,2452,2453,2770,2771,2772,2775,2776,2835,2836,2838,3355,3356,3357,3358,3369,3470,3478,3486,3818,3819,3820,3821,3857,3858,4625,7911,7912,7966,8153,8831,8836,8838,8839,8845,8846,10620,11370,12365,12644,12809,13463,13464,13465,13466,13467,13468,19726,19727,22202,22203,22710,22785,22786,22787,22788,22789,22790,22791,22792,22793,22794,22797,23424,23425,23426,23427,35128,36901,36902,36903,36904,36905,36906,36907,36908,36909,36910,36912,37921) AND `Entry` IN (11458,11459,11461,11464,11465,13021,13196,13197,13285,14303,15637,9601);
DELETE FROM `creature_loot_template` WHERE `Item` IN (765,785,2447,2449,2450,2452,2453,2770,2771,2772,2775,2776,2835,2836,2838,3355,3356,3357,3358,3369,3470,3478,3486,3818,3819,3820,3821,3857,3858,4625,7911,7912,7966,8153,8831,8836,8838,8839,8845,8846,10620,11370,12365,12644,12809,13463,13464,13465,13466,13467,13468,19726,19727,22202,22203,22710,22785,22786,22787,22788,22789,22790,22791,22792,22793,22794,22797,23424,23425,23426,23427,35128,36901,36902,36903,36904,36905,36906,36907,36908,36909,36910,36912,37921) AND `Entry` IN (678,679,680,709,710,2564,2573,2574,2719,2720,3056,3058,3068,3655,4131,4132,4625,4645);
