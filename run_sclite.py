import sys, subprocess, os

expt_dir = sys.argv[1]
results_dir = sys.argv[2]
ref_fpath = sys.argv[3]
expt_name = os.path.basename(expt_dir)


print 'Prepping sclite...'

os.chdir(results_dir)
expt_results_dir = '{}_results'.format(expt_name)
if not os.path.exists(expt_results_dir):
	os.mkdir(expt_results_dir)
os.chdir(expt_results_dir)

hyp_path = '{}_temp.hyp'.format(expt_name)
output_dir = os.path.join(expt_dir, 'output', 'txt')
with open(hyp_path, 'w') as hypfile:
	hyp_list = []
	for f in os.listdir(output_dir):
		fpath = os.path.join(output_dir, f)
		with open(fpath) as transcript:
			trs_text = transcript.read().lower().strip()
			idname = ' ({}-1)'.format(f[:-4])
			trs_text += idname
			hyp_list.append(trs_text)
		hyp_str = '\n'.join(hyp_list)
	hypfile.write(hyp_str)
	hypfile.write('\n')

final_hyp_path = '{}.hyp'.format(expt_name)
os.system("cat {} | tr -d '.' > {}".format(hyp_path, final_hyp_path))

print 'Running sclite...'
os.system('sclite -r {0} -h {1}.hyp -n {1} -i rm -o dtl pra sum'.format(ref_fpath, expt_name))

print 'Done.'











