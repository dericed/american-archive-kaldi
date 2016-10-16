# Pop Up Archive Kaldi release
## Prequisites
* [Kaldi] (https://github.com/kaldi-asr/kaldi)
* [SoX] (http://sox.sourceforge.net/)
* CMUSeg (use `sample_experiment/install-cmuseg.sh`)
* IRSTLM (use `[KALDI-PATH]/tools/extras/install_irstlm.sh`)
* sclite (see `[KALDI-PATH]/tools/sctk-2.4.10`)
* [exp dir] (https://sourceforge.net/projects/popuparchive-kaldi/files/)
* ftfy (do `pip install ftfy`)

##Notes about Kaldi
It is recommended that you review the [Kaldi documentation] (http://kaldi-asr.org/doc/) before you begin, especially if you intend to modify the compiled model included on Sourceforge.

The models linked to from this repo are trained on public media content from NPR and many podcasts. This software is intended specifically for the transcription of public media content, though anyone is welcome to use it for content of their choosing.

Read more about this software: https://docs.google.com/presentation/d/1ZL0Qg-tsAp6zg95IEKit8fgtf5kISTriUjKe0UrbG7E/edit?usp=sharing

## Expected Set-up
Each model you experiment with should have its own directory. Start by putting the exp dir from Sourceforge in the `sample_experiment` dir.
### Preliminaries for your experiment dir (e.g. `sample_experiment`)
* Make sure `path.sh` and `set-kaldi-path.sh` match your Kaldi location.
* Create the following sym links in your experiment dir
	* `ln -s [KALDI-PATH]/egs/wsj/s5/steps [EXPT-DIR]/exp`
	* `ln -s [KALDI-PATH]/egs/wsj/s5/utils [EXPT-DIR]/exp`

### Other preliminaries
* Create a directory to store results from sclite, e.g. from your main dir, do `mkdir results`

## Usage
Run kaldi speech recognition on directory of wav files: 
`python run_kaldi.py [EXPERIMENT-DIR] [WAV-DIR]`  
Run evaluation:
`python run_sclite.py [EXPERIMENT-DIR] [RESULTS-DIR] [REF-FILE-PATH]`

To view the summary results, open `[RESULTS-DIR]/[EXP-RESULTS-DIR]/[EXP-NAME]_results.sys`

##Building on the model
You can use the current model as is, or add your own lexicon or language model.
* You can add new words to the lexicon by editing `exp/dict/lexicon.txt` and running `sh prep_lang_local.sh exp/dict exp/tmp_lang exp/lang`. 
* Use `sh add_grammar.sh [LM-FILEPATH]` to create a new language model based on an LM textfile and to update the overall model.

## Acknowledgements
The development of this software was funded by an Institute of Museum and Library Services Research Grant to WGBH for the “Improving Access to Time Based Media through Crowdsourcing and Machine Learning” project. 

Read more here: https://americanarchivepb.wordpress.com/2015/09/14/imls-aapb-pop-up-archive-grant/
See the full proposal here: https://www.imls.gov/sites/default/files/proposal_narritive_lg-71-15-0208_wgbh_educational_foundation.pdf

This repo is based on [work] (https://github.com/APMG/audio-search) done by APM with Cantab Research.
