import sys, re

lm_text_fpath = sys.argv[1] #training text with one utt per line
wordlist_fpath = sys.argv[2] #file with one word per line
out_fpath = sys.argv[3] #file to write to

def convert_line(wordlist, line):
	orig_linewords = line.lower().split()
	new_linewords = []
	for w in orig_linewords:
		if w in wordlist:
			new_linewords.append(w)
		else:
			new_linewords.append('<unk>')
	return ' '.join(new_linewords)

with open(wordlist_fpath) as words_file:
	iv_words = set(words_file.read().split())

lm_lines = []
with open(lm_text_fpath) as lm_file:
	for line in lm_file.readlines():
		lm_lines.append(convert_line(iv_words, line))

with open(out_fpath, 'w') as out_file:
	out_file.write('\n'.join(lm_lines))


