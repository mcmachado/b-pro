import sys

list_of_games = [
'asterix', 'beam_rider', 'breakout', 'enduro', 'freeway', 'pong', 'qbert', 'seaquest', 'space_invaders',
'private_eye', 
'alien', 'amidar', 'assault', 'asteroids', 'atlantis', 
'bank_heist', 'battle_zone', 'berzerk', 'bowling', 'boxing',
'carnival', 'centipede', 'chopper_command', 'crazy_climber',
'demon_attack', 'double_dunk',
'elevator_action',
'fishing_derby', 'frostbite',
'gopher', 'gravitar',
'hero',
'ice_hockey',
'jamesbond','journey_escape',
'kangaroo', 'krull', 'kung_fu_master',
'montezuma_revenge', 'ms_pacman',
'name_this_game',
'pooyan',
'riverraid', 'road_runner', 'robotank',
'skiing', 'star_gunner',
'tennis', 'time_pilot', 'tutankham',
'up_n_down',
'venture', 'video_pinball',
'wizard_of_wor',
'zaxxon']

print len(list_of_games)
def generatePBSFiles(game_name):
	for i in xrange(24):
		f = open(game_name + '_' + str(i+1) + '.pbs', 'w')
		f.write('#!/bin/bash\n')
		f.write('#PBS -S /bin/bash\n\n')
		f.write('# Script for running\n\n')

		f.write('#PBS -l nodes=1\n')
		f.write('#PBS -l mem=1250mb\n')
		f.write('#PBS -l walltime=120:00:00\n')
		f.write('#PBS -M machado@cs.ualberta.ca\n\n')

		f.write('LD_LIBRARY_PATH=../../Arcade-Learning-Environment/\n')
		f.write('export LD_LIBRARY_PATH\n\n')
		f.write('cd $PBS_O_WORKDIR\n\n')
		f.write('./learnerBpro -c bpro.cfg -r ../roms/' + game_name + '.bin -s ' + str(i+1) + ' -n $PARALLEL_SCRATCH_MP2_WIPE_ON_AUGUST_2016/checkPoints_full/' + game_name + '/' + str(i+1) + ' > $PARALLEL_SCRATCH_MP2_WIPE_ON_AUGUST_2016/results_full/' + game_name + '/' + str(i+1) + '.out\n')

		f.close()

f = open('submitEverything.sh', 'w')
for game_name in list_of_games:
	generatePBSFiles(game_name)
	f.write('mkdir $PARALLEL_SCRATCH_MP2_WIPE_ON_AUGUST_2016/results_full/' + game_name + '\n')
        f.write('mkdir $PARALLEL_SCRATCH_MP2_WIPE_ON_AUGUST_2016/checkPoints_full/' + game_name + '\n')
	f.write('echo Submitting ' + game_name + '...\n')
	for i in xrange(24):
		f.write('bqsub ' + game_name + '_' + str(i+1) + '.pbs\n')
#	f.write('echo Submitting ' + game_name + '...\n')
#	f.write('bqsub --submit\n')

f.close()
