from dmTxt2Html import *
import glob

txts = glob.glob('*.txt')
cfg_dict = {'type': 'SMART_ASCII',
            'proxy': 'NONE' }
for fname in txts:
    cfg_dict['source'] = fname
    cfg_dict['target'] = fname[:-3]+'html'
    main(cfg_dict)


