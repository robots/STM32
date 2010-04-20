#!/usr/bin/env python

"""Create full-text indexes and search them

Notes:

  See http://gnosis.cx/publish/programming/charming_python_15.txt
  for a detailed discussion of this module.

  This version requires Python 1.6+.  It turns out that the use
  of string methods rather than [string] module functions is
  enough faster in a tight loop so as to provide a quite
  remarkable 25% speedup in overall indexing.  However, only FOUR
  lines in TextSplitter.text_splitter() were changed away from
  Python 1.5 compatibility.  Those lines are followed by comments
  beginning with "# 1.52:  " that show the old forms.  Python
  1.5 users can restore these lines, and comment out those just
  above them.

Classes:

    GenericIndexer      -- Abstract class
    TextSplitter        -- Mixin class
    Index
    ShelveIndexer
    FlatIndexer
    XMLPickleIndexer
    PickleIndexer
    ZPickleIndexer
    SlicedZPickleIndexer

Functions:

    echo_fname(fname)
    recurse_files(...)

Index Formats:

    *Indexer.files:     filename --> (fileid, wordcount)
    *Indexer.fileids:   fileid --> filename
    *Indexer.words:     word --> {fileid1:occurs, fileid2:occurs, ...}

Module Usage:

  There are a few ways to use this module.  Just to utilize existing
  functionality, something like the following is a likely
  pattern:

      import gnosis.indexer as indexer
      index = indexer.MyFavoriteIndexer()     # For some concrete Indexer
      index.load_index('myIndex.db')
      index.add_files(dir='/this/that/otherdir', pattern='*.txt')
      hits = index.find(['spam','eggs','bacon'])
      index.print_report(hits)

  To customize the basic classes, something like the following is likely:

      class MySplitter:
          def splitter(self, text, ftype):
              "Peform much better splitting than default (for filetypes)"
              # ...
              return words

      class MyIndexer(indexer.GenericIndexer, MySplitter):
          def load_index(self, INDEXDB=None):
              "Retrieve three dictionaries from clever storage method"
              # ...
              self.words, self.files, self.fileids = WORDS, FILES, FILEIDS
          def save_index(self, INDEXDB=None):
              "Save three dictionaries to clever storage method"

      index = MyIndexer()
      # ...etc...

Benchmarks:

  As we know, there are lies, damn lies, and benchmarks.  Take
  the below with an adequate dose of salt.  In version 0.10 of
  the concrete indexers, some performance was tested.  The
  test case was a set of mail/news archives, that were about
  43 mB, and 225 files.  In each case, an index was generated
  (if possible), and a search for the words "xml python" was
  performed.

    - Index w/ PickleIndexer:     482s, 2.4 mB
    - Search w/ PickleIndexer:    1.74s
    - Index w/ ZPickleIndexer:    484s, 1.2 mB
    - Search w/ ZPickleIndexer:   1.77s
    - Index w/ FlatIndexer:       492s, 2.6 mB
    - Search w/ FlatIndexer:      53s
    - Index w/ ShelveIndexer:     (dumbdbm) Many minutes, tens of mBs
    - Search w/ ShelveIndexer:    Aborted before completely indexed
    - Index w/ ShelveIndexer:     (dbhash) Long time (partial crash), 10 mB
    - Search w/ ShelveIndexer:    N/A. Too many glitches
    - Index w/ XMLPickleIndexer:  Memory error (xml_pickle uses bad string
                                                composition for large output)
    - Search w/ XMLPickleIndexer: N/A
    - grep search (xml|python):   20s (cached: <5s)
    - 'srch' utility (python):    12s
"""
__shell_usage__ = """
Shell Usage: [python] indexer.py [options] [search_words]

    -h, /h, -?, /?, ?, --help:    Show this help screen
    -index:                       Add files to index
    -reindex:                     Refresh files already in the index
                                  (can take much more time)
    -casesensitive:               Maintain the case of indexed words
                                  (can lead to MUCH larger indices)
    -norecurse, -local:           Only index starting dir, not subdirs
    -dir=<directory>:             Starting directory for indexing
                                  (default is current directory)
    -indexdb=<database>:          Use specified index database
                                  (environ variable INDEXER_DB is preferred)
    -regex=<pattern>:             Index files matching regular expression
    -glob=<pattern>:              Index files matching glob pattern
    -filter=<pattern>             Only display results matching pattern
    -output=<op>, -format=<opt>:  How much detail on matches?
    -<digit>:                     Quiet level (0=verbose ... 9=quiet)

Output/format options are ALL/EVERYTHING/VERBOSE, RATINGS/SCORES,
FILENAMES/NAMES/FILES, SUMMARY/REPORT"""

__version__ = "August 2002"
__author__=["David Mertz (mertz@gnosis.cx)",]
__thanks_to__=["Pat Knight (p.knight@ktgroup.co.uk)",
               "Gregory Popovitch (greg@gpy.com)",
               "Leonard Richardson (leonardr@segfault.org)", ]
__copyright__="""
    This file is released to the public domain.  I (dqm) would
    appreciate it if you choose to keep derived works under terms
    that promote freedom, but obviously am giving up any rights
    to compel such.
"""

__history__="""
    0.1    Initial version.

    0.11   Tweaked TextSplitter after some random experimentation.

    0.12   Added SlicedZPickleIndexer (best choice, so far).

    0.13   Pat Knight pointed out need for binary open()'s of
           certain files under Windows.

    0.14   Added '-filter' switch to search results.

    0.15   Added direct read of gzip files

    0.20   Gregory Popovitch did some profiling on TextSplitter,
           and provided both huge speedups to the Python version
           and hooks to a C extension class (ZopeTextSplitter).
           A little refactoring by he and I (dqm) has nearly
           doubled the speed of indexing

    0.30   Module refactored into gnosis package.  This is a
           first pass, and various documentation and test cases
           should be added later.

    08/02  Leonard Richardson corrected a bug with reindexing
           fileids already in the hash.
"""
import string, re, os, fnmatch, sys, copy, gzip
from types import *

#-- Silly "do nothing" default recursive file processor
def echo_fname(fname): print fname

#-- "Recurse and process files" utility function
def recurse_files(curdir, pattern, exclusions, func=echo_fname, *args, **kw):
    "Recursively process file pattern"
    subdirs, files = [],[]
    level = kw.get('level',0)

    for name in os.listdir(curdir):
        fname = os.path.join(curdir, name)
        if name[-4:] in exclusions:
            pass            # do not include binary file type
        elif os.path.isdir(fname) and not os.path.islink(fname):
            subdirs.append(fname)
        # is it a regular expression?
        elif type(pattern)==type(re.compile('')):
            if pattern.match(name):
                files.append(fname)
        elif type(pattern) is StringType:
            if fnmatch.fnmatch(name, pattern):
                files.append(fname)

    for fname in files:
        apply(func, (fname,)+args)
    for subdir in subdirs:
        recurse_files(subdir, pattern, exclusions, func, level=level+1)

#-- Data bundle for index dictionaries
class Index:
    def __init__(self, words, files, fileids):
        if words is not None:   self.WORDS = words
        if files is not None:   self.FILES = files
        if fileids is not None: self.FILEIDS = fileids

#-- "Split plain text into words" utility function
class TextSplitter:
    def initSplitter(self):
        prenum  = string.join(map(chr, range(0,48)), '')
        num2cap = string.join(map(chr, range(58,65)), '')
        cap2low = string.join(map(chr, range(91,97)), '')
        postlow = string.join(map(chr, range(123,256)), '')
        nonword = prenum + num2cap + cap2low + postlow
        self.word_only = string.maketrans(nonword, " "*len(nonword))
        self.nondigits = string.join(map(chr, range(0,48)) + map(chr, range(58,255)), '')
        self.alpha = string.join(map(chr, range(65,91)) + map(chr, range(97,123)), '')
        self.ident = string.join(map(chr, range(256)), '')
        self.init = 1

    def splitter(self, text, ftype):
        "Split the contents of a text string into a list of 'words'"
        if ftype == 'text/plain':
            words = self.text_splitter(text, self.casesensitive)
        else:
            raise NotImplementedError
        return words

    def text_splitter(self, text, casesensitive=0):
        """Split text/plain string into a list of words

        In version 0.20 this function is still fairly weak at
        identifying "real" words, and excluding gibberish
        strings.  As long as the indexer looks at "real" text
        files, it does pretty well; but if indexing of binary
        data is attempted, a lot of gibberish gets indexed.
        Suggestions on improving this are GREATLY APPRECIATED.
        """
        # Initialize some constants
        if not hasattr(self,'init'): self.initSplitter()

        # Speedup trick: attributes into local scope
        word_only = self.word_only
        ident = self.ident
        alpha = self.alpha
        nondigits = self.nondigits
        # 1.52: translate = string.translate

        # Let's adjust case if not case-sensitive
        if not casesensitive: text = string.upper(text)

        # Split the raw text
        allwords = string.split(text)

        # Finally, let's skip some words not worth indexing
        words = []
        for word in allwords:
            if len(word) > 25: continue         # too long (probably gibberish)

            # Identify common patterns in non-word data (binary, UU/MIME, etc)
            num_nonalpha = len(word.translate(ident, alpha))
            numdigits    = len(word.translate(ident, nondigits))
            # 1.52: num_nonalpha = len(translate(word, ident, alpha))
            # 1.52: numdigits    = len(translate(word, ident, nondigits))
            if numdigits > len(word)-2:         # almost all digits
                if numdigits > 5:               # too many digits is gibberish
                    continue                    # a moderate number is year/zipcode/etc
            elif num_nonalpha*3 > len(word):    # too much scattered nonalpha = gibberish
                continue

            word = word.translate(word_only)    # Let's strip funny byte values
            # 1.52: word = translate(word, word_only)
            subwords = word.split()             # maybe embedded non-alphanumeric
            # 1.52: subwords = string.split(word)
            for subword in subwords:            # ...so we might have subwords
                if len(subword) <= 2: continue  # too short a subword
                words.append(subword)
        return words

class  ZopeTextSplitter:
    def initSplitter(self):
        import Splitter
        stop_words=(
            'am', 'ii', 'iii', 'per', 'po', 're', 'a', 'about', 'above', 'across',
            'after', 'afterwards', 'again', 'against', 'all', 'almost', 'alone',
            'along', 'already', 'also', 'although', 'always', 'am', 'among',
            'amongst', 'amoungst', 'amount', 'an', 'and', 'another', 'any',
            'anyhow', 'anyone', 'anything', 'anyway', 'anywhere', 'are', 'around',
            'as', 'at', 'back', 'be', 'became', 'because', 'become', 'becomes',
            'becoming', 'been', 'before', 'beforehand', 'behind', 'being',
            'below', 'beside', 'besides', 'between', 'beyond', 'bill', 'both',
            'bottom', 'but', 'by', 'can', 'cannot', 'cant', 'con', 'could',
            'couldnt', 'cry', 'describe', 'detail', 'do', 'done', 'down', 'due',
            'during', 'each', 'eg', 'eight', 'either', 'eleven', 'else',
            'elsewhere', 'empty', 'enough', 'even', 'ever', 'every', 'everyone',
            'everything', 'everywhere', 'except', 'few', 'fifteen', 'fifty',
            'fill', 'find', 'fire', 'first', 'five', 'for', 'former', 'formerly',
            'forty', 'found', 'four', 'from', 'front', 'full', 'further', 'get',
            'give', 'go', 'had', 'has', 'hasnt', 'have', 'he', 'hence', 'her',
            'here', 'hereafter', 'hereby', 'herein', 'hereupon', 'hers',
            'herself', 'him', 'himself', 'his', 'how', 'however', 'hundred', 'i',
            'ie', 'if', 'in', 'inc', 'indeed', 'interest', 'into', 'is', 'it',
            'its', 'itself', 'keep', 'last', 'latter', 'latterly', 'least',
            'less', 'made', 'many', 'may', 'me', 'meanwhile', 'might', 'mill',
            'mine', 'more', 'moreover', 'most', 'mostly', 'move', 'much', 'must',
            'my', 'myself', 'name', 'namely', 'neither', 'never', 'nevertheless',
            'next', 'nine', 'no', 'nobody', 'none', 'noone', 'nor', 'not',
            'nothing', 'now', 'nowhere', 'of', 'off', 'often', 'on', 'once',
            'one', 'only', 'onto', 'or', 'other', 'others', 'otherwise', 'our',
            'ours', 'ourselves', 'out', 'over', 'own', 'per', 'perhaps',
            'please', 'pre', 'put', 'rather', 're', 'same', 'see', 'seem',
            'seemed', 'seeming', 'seems', 'serious', 'several', 'she', 'should',
            'show', 'side', 'since', 'sincere', 'six', 'sixty', 'so', 'some',
            'somehow', 'someone', 'something', 'sometime', 'sometimes',
            'somewhere', 'still', 'such', 'take', 'ten', 'than', 'that', 'the',
            'their', 'them', 'themselves', 'then', 'thence', 'there',
            'thereafter', 'thereby', 'therefore', 'therein', 'thereupon', 'these',
            'they', 'thick', 'thin', 'third', 'this', 'those', 'though', 'three',
            'through', 'throughout', 'thru', 'thus', 'to', 'together', 'too',
            'toward', 'towards', 'twelve', 'twenty', 'two', 'un', 'under',
            'until', 'up', 'upon', 'us', 'very', 'via', 'was', 'we', 'well',
            'were', 'what', 'whatever', 'when', 'whence', 'whenever', 'where',
            'whereafter', 'whereas', 'whereby', 'wherein', 'whereupon',
            'wherever', 'whether', 'which', 'while', 'whither', 'who', 'whoever',
            'whole', 'whom', 'whose', 'why', 'will', 'with', 'within', 'without',
            'would', 'yet', 'you', 'your', 'yours', 'yourself', 'yourselves',
            )
        self.stop_word_dict={}
        for word in stop_words: self.stop_word_dict[word]=None
        self.splitterobj = Splitter.getSplitter()
        self.init = 1

    def goodword(self, word):
        return len(word) < 25

    def splitter(self, text, ftype):
        """never case-sensitive"""
        if not hasattr(self,'init'): self.initSplitter()
        return filter(self.goodword, self.splitterobj(text, self.stop_word_dict))


#-- "Abstract" parent class for inherited indexers
#   (does not handle storage in parent, other methods are primitive)

class GenericIndexer:
    def __init__(self, **kw):
        apply(self.configure, (), kw)

    def whoami(self):
        return self.__class__.__name__

    def configure(self, REINDEX=0, CASESENSITIVE=0,
                        INDEXDB=os.environ.get('INDEXER_DB', 'TEMP_NDX.DB'),
                        ADD_PATTERN='*', QUIET=5):
        "Configure settings used by indexing and storage/retrieval"
        self.indexdb = INDEXDB
        self.reindex = REINDEX
        self.casesensitive = CASESENSITIVE
        self.add_pattern = ADD_PATTERN
        self.quiet = QUIET
        self.filter = None

    def add_files(self, dir=os.getcwd(), pattern=None, descend=1):
        self.load_index()
        exclusions = ('.zip','.pyc','.gif','.jpg','.dat','.dir')
        if not pattern:
             pattern = self.add_pattern
        recurse_files(dir, pattern, exclusions, self.add_file)

    def add_file(self, fname, ftype='text/plain'):
        "Index the contents of a regular file"
        if self.files.has_key(fname):   # Is file eligible for (re)indexing?
            if self.reindex:            # Reindexing enabled, cleanup dicts
                self.purge_entry(fname, self.fileids, self.files, self.words)
            else:                   # DO NOT reindex this file
                if self.quiet < 5: print "Skipping", fname
                return 0

        # Read in the file (if possible)
        try:
            if fname[-3:] == '.gz':
                text = gzip.open(fname).read()
            else:
                text = open(fname).read()
            if self.quiet < 5: print "Indexing", fname
        except IOError:
            return 0
        words = self.splitter(text, ftype)

        # Find new file index, and assign it to filename
        # (_TOP uses trick of negative to avoid conflict with file index)
        self.files['_TOP'] = (self.files['_TOP'][0]-1, None)
        file_index =  abs(self.files['_TOP'][0])
        self.files[fname] = (file_index, len(words))
        self.fileids[file_index] = fname

        filedict = {}
        for word in words:
            if filedict.has_key(word):
                filedict[word] = filedict[word]+1
            else:
                filedict[word] = 1

        for word in filedict.keys():
            if self.words.has_key(word):
                entry = self.words[word]
            else:
                entry = {}
            entry[file_index] = filedict[word]
            self.words[word] = entry

    def add_othertext(self, identifier):
        """Index a textual source other than a plain file

        A child class might want to implement this method (or a similar one)
        in order to index textual sources such as SQL tables, URLs, clay
        tablets, or whatever else.  The identifier should uniquely pick out
        the source of the text (whatever it is)
        """
        raise NotImplementedError

    def save_index(self, INDEXDB=None):
        raise NotImplementedError

    def load_index(self, INDEXDB=None, reload=0, wordlist=None):
        raise NotImplementedError

    def find(self, wordlist, print_report=0):
        "Locate files that match ALL the words in wordlist"
        self.load_index(wordlist=wordlist)
        entries = {}
        hits = copy.copy(self.fileids)      # Copy of fileids index
        for word in wordlist:
            if not self.casesensitive:
                word = string.upper(word)
            entry = self.words.get(word)    # For each word, get index
            entries[word] = entry           #   of matching files
            if not entry:                   # Nothing for this one word (fail)
                return 0
            for fileid in hits.keys():      # Eliminate hits for every non-match
                if not entry.has_key(fileid):
                    del hits[fileid]
        if print_report:
            self.print_report(hits, wordlist, entries)
        return hits

    def print_report(self, hits={}, wordlist=[], entries={}):
        # Figure out what to actually print (based on QUIET level)
        output = []
        for fileid,fname in hits.items():
            message = fname
            if self.quiet <= 3:
                wordcount = self.files[fname][1]
                matches = 0
                countmess = '\n'+' '*13+`wordcount`+' words; '
                for word in wordlist:
                    if not self.casesensitive:
                        word = string.upper(word)
                    occurs = entries[word][fileid]
                    matches = matches+occurs
                    countmess = countmess +`occurs`+' '+word+'; '
                message = string.ljust('[RATING: '
                                       +`1000*matches/wordcount`+']',13)+message
                if self.quiet <= 2: message = message +countmess +'\n'
            if self.filter:     # Using an output filter
                if fnmatch.fnmatch(message, self.filter):
                    output.append(message)
            else:
                output.append(message)

        if self.quiet <= 5:
            print string.join(output,'\n')
        sys.stderr.write('\n'+`len(output)`+' files matched wordlist: '+
                         `wordlist`+'\n')
        return output

    def purge_entry(self, fname, file_ids, file_dct, word_dct):
        "Remove a file from file index and word index"
        try:        # The easy part, cleanup the file index
            file_index = file_dct[fname]
            del file_dct[fname]
            del file_ids[file_index[0]]
        except KeyError:
            pass    # We'll assume we only encounter KeyError's
        # The much harder part, cleanup the word index
        for word, occurs in word_dct.items():
            if occurs.has_key(file_index):
                del occurs[file_index]
                word_dct[word] = occurs

    def index_loaded(self):
        return ( hasattr(self,'fileids') and
                 hasattr(self,'files')   and
                 hasattr(self,'words')      )

#-- Provide an actual storage facility for the indexes (i.e. shelve)
class ShelveIndexer(GenericIndexer, TextSplitter):
    """Concrete Indexer utilizing [shelve] for storage

    Unfortunately, [shelve] proves far too slow in indexing, while
    creating monstrously large indexes.  Not recommend, at least under
    the default dbm's tested.  Also, class may be broken because
    shelves do not, apparently, support the .values() and .items()
    methods.  Fixing this is a low priority, but the sample code is
    left here.
    """
    def load_index(self, INDEXDB=None, reload=0, wordlist=None):
        INDEXDB = INDEXDB or self.indexdb
        import shelve
        self.words   = shelve.open(INDEXDB+".WORDS")
        self.files   = shelve.open(INDEXDB+".FILES")
        self.fileids = shelve.open(INDEXDB+".FILEIDS")
        if not FILES:            # New index
            self.files['_TOP'] = (0,None)

    def save_index(self, INDEXDB=None):
        INDEXDB = INDEXDB or self.indexdb
        pass

class FlatIndexer(GenericIndexer, TextSplitter):
    """Concrete Indexer utilizing flat-file for storage

    See the comments in the referenced article for details; in
    brief, this indexer has about the same timing as the best in
    -creating- indexes and the storage requirements are
    reasonable.  However, actually -using- a flat-file index is
    more than an order of magnitude worse than the best indexer
    (ZPickleIndexer wins overall).

    On the other hand, FlatIndexer creates a wonderfully easy to
    parse database format if you have a reason to transport the
    index to a different platform or programming language.  And
    should you perform indexing as part of a long-running
    process, the overhead of initial file parsing becomes
    irrelevant.
    """
    def load_index(self, INDEXDB=None, reload=0, wordlist=None):
        # Unless reload is indicated, do not load twice
        if self.index_loaded() and not reload: return 0
        # Ok, now let's actually load it
        INDEXDB = INDEXDB or self.indexdb
        self.words = {}
        self.files = {'_TOP':(0,None)}
        self.fileids = {}
        try:                            # Read index contents
            for line in open(INDEXDB).readlines():
                fields = string.split(line)
                if fields[0] == '-':    # Read a file/fileid line
                    fileid = eval(fields[2])
                    wordcount = eval(fields[3])
                    fname = fields[1]
                    self.files[fname] = (fileid, wordcount)
                    self.fileids[fileid] = fname
                else:                   # Read a word entry (dict of hits)
                    entries = {}
                    word = fields[0]
                    for n in range(1,len(fields),2):
                        fileid = eval(fields[n])
                        occurs = eval(fields[n+1])
                        entries[fileid] = occurs
                    self.words[word] = entries
        except:
            pass                    # New index

    def save_index(self, INDEXDB=None):
        INDEXDB = INDEXDB or self.indexdb
        tab, lf, sp = '\t','\n',' '
        indexdb = open(INDEXDB,'w')
        for fname,entry in self.files.items():
            indexdb.write('- '+fname +tab +`entry[0]` +tab +`entry[1]` +lf)
        for word,entry in self.words.items():
            indexdb.write(word +tab+tab)
            for fileid,occurs in entry.items():
                indexdb.write(`fileid` +sp +`occurs` +sp)
            indexdb.write(lf)

class PickleIndexer(GenericIndexer, TextSplitter):
    def load_index(self, INDEXDB=None, reload=0, wordlist=None):
        # Unless reload is indicated, do not load twice
        if self.index_loaded() and not reload: return 0
        # Ok, now let's actually load it
        import cPickle
        INDEXDB = INDEXDB or self.indexdb
        try:
            pickle_str =  open(INDEXDB,'rb').read()
            db = cPickle.loads(pickle_str)
        except:                     # New index
            db = Index({}, {'_TOP':(0,None)}, {})
        self.words, self.files, self.fileids = db.WORDS, db.FILES, db.FILEIDS

    def save_index(self, INDEXDB=None):
        import cPickle
        INDEXDB = INDEXDB or self.indexdb
        db = Index(self.words, self.files, self.fileids)
        open(INDEXDB,'wb').write(cPickle.dumps(db, 1))

class XMLPickleIndexer(PickleIndexer):
    """Concrete Indexer utilizing XML for storage

    While this is, as expected, a verbose format, the possibility
    of using XML as a transport format for indexes might be
    useful.  However, [xml_pickle] is in need of some redesign to
    avoid gross inefficiency when creating very large
    (multi-megabyte) output files (fixed in [xml_pickle] version
    0.48 or above)
    """
    def load_index(self, INDEXDB=None, reload=0, wordlist=None):
        # Unless reload is indicated, do not load twice
        if self.index_loaded() and not reload: return 0
        # Ok, now let's actually load it
        from gnosis.xml.pickle import XML_Pickler
        INDEXDB = INDEXDB or self.indexdb
        try:                        # XML file exists
            xml_str = open(INDEXDB).read()
            db = XML_Pickler().loads(xml_str)
        except:                     # New index
            db = Index({}, {'_TOP':(0,None)}, {})
        self.words, self.files, self.fileids = db.WORDS, db.FILES, db.FILEIDS

    def save_index(self, INDEXDB=None):
        from gnosis.xml.pickle import XML_Pickler
        INDEXDB = INDEXDB or self.indexdb
        db = Index(self.words, self.files, self.fileids)
        open(INDEXDB,'w').write(XML_Pickler(db).dumps())

class ZPickleIndexer(PickleIndexer):
    def load_index(self, INDEXDB=None, reload=0, wordlist=None):
        # Unless reload is indicated, do not load twice
        if self.index_loaded() and not reload: return 0
        # Ok, now let's actually load it
        import cPickle, zlib
        INDEXDB = INDEXDB or self.indexdb
        try:
            pickle_str =  zlib.decompress(open(INDEXDB+'!','rb').read())
            db = cPickle.loads(pickle_str)
        except:                     # New index
            db = Index({}, {'_TOP':(0,None)}, {})
        self.words, self.files, self.fileids = db.WORDS, db.FILES, db.FILEIDS

    def save_index(self, INDEXDB=None):
        import cPickle, zlib
        INDEXDB = INDEXDB or self.indexdb
        db = Index(self.words, self.files, self.fileids)
        pickle_fh = open(INDEXDB+'!','wb')
        pickle_fh.write(zlib.compress(cPickle.dumps(db, 1)))


class SlicedZPickleIndexer(ZPickleIndexer):
    segments = "ABCDEFGHIJKLMNOPQRSTUVWXYZ#-!"
    def load_index(self, INDEXDB=None, reload=0, wordlist=None):
        # Unless reload is indicated, do not load twice
        if self.index_loaded() and not reload: return 0
        # Ok, now let's actually load it
        import cPickle, zlib
        INDEXDB = INDEXDB or self.indexdb
        db = Index({}, {'_TOP':(0,None)}, {})
        # Identify the relevant word-dictionary segments
        if not wordlist:
            segments = self.segments
        else:
            segments = ['-','#']
            for word in wordlist:
                segments.append(string.upper(word[0]))
        # Load the segments
        for segment in segments:
            try:
                pickle_str = zlib.decompress(open(INDEXDB+segment,'rb').read())
                dbslice = cPickle.loads(pickle_str)
                if dbslice.__dict__.get('WORDS'):   # If it has some words, add them
                    for word,entry in dbslice.WORDS.items():
                        db.WORDS[word] = entry
                if dbslice.__dict__.get('FILES'):   # If it has some files, add them
                    db.FILES = dbslice.FILES
                if dbslice.__dict__.get('FILEIDS'): # If it has fileids, add them
                    db.FILEIDS = dbslice.FILEIDS
            except:
                pass    # No biggie, couldn't find this segment
        self.words, self.files, self.fileids = db.WORDS, db.FILES, db.FILEIDS

    def julienne(self, INDEXDB=None):
        import cPickle, zlib
        INDEXDB = INDEXDB or self.indexdb
        segments = self.segments       # all the (little) indexes
        for segment in segments:
            try:        # brutal space saver... delete all the small segments
                os.remove(INDEXDB+segment)
            except OSError:
                pass    # probably just nonexistent segment index file
        # First write the much simpler filename/fileid dictionaries
        dbfil = Index(None, self.files, self.fileids)
        open(INDEXDB+'-','wb').write(zlib.compress(cPickle.dumps(dbfil,1)))
        # The hard part is splitting the word dictionary up, of course
        letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        segdicts = {}                           # Need batch of empty dicts
        for segment in letters+'#':
            segdicts[segment] = {}
        for word, entry in self.words.items():  # Split into segment dicts
            initchar = string.upper(word[0])
            if initchar in letters:
                segdicts[initchar][word] = entry
            else:
                segdicts['#'][word] = entry
        for initchar in letters+'#':
            db = Index(segdicts[initchar], None, None)
            pickle_str = cPickle.dumps(db, 1)
            pickle_fh = open(INDEXDB+initchar,'wb')
            pickle_fh.write(zlib.compress(pickle_str))

    save_index = julienne

PreferredIndexer = SlicedZPickleIndexer

#-- If called from command-line, parse arguments and take actions
if __name__ == '__main__':
    import time
    start = time.time()
    search_words = []           # Word search list (if specified)
    opts = 0                    # Any options specified?
    if len(sys.argv) < 2:
        pass                    # No options given
    else:
        upper = string.upper
        dir = os.getcwd()       # Default to indexing from current directory
        descend = 1             # Default to recursive indexing
        ndx = PreferredIndexer()
        for opt in sys.argv[1:]:
            if opt in ('-h','/h','-?','/?','?','--help'):   # help screen
                print __shell_usage__
                opts = -1
                break
            elif opt[0] in '/-':                            # a switch!
                opts = opts+1
                if upper(opt[1:]) == 'INDEX':               # Index files
                    ndx.quiet = 0
                    pass     # Use defaults if no other options
                elif upper(opt[1:]) == 'REINDEX':           # Reindex
                    ndx.reindex = 1
                elif upper(opt[1:]) == 'CASESENSITIVE':     # Case sensitive
                    ndx.casesensitive = 1
                elif upper(opt[1:]) in ('NORECURSE','LOCAL'): # No recursion
                    descend = 0
                elif upper(opt[1:4]) == 'DIR':              # Dir to index
                    dir = opt[5:]
                elif upper(opt[1:8]) == 'INDEXDB':          # Index specified
                    ndx.indexdb = opt[9:]
                    sys.stderr.write(
                        "Use of INDEXER_DB environment variable is STRONGLY recommended.\n")
                elif upper(opt[1:6]) == 'REGEX':            # RegEx files to index
                    ndx.add_pattern = re.compile(opt[7:])
                elif upper(opt[1:5]) == 'GLOB':             # Glob files to index
                    ndx.add_pattern = opt[6:]
                elif upper(opt[1:7]) in ('OUTPUT','FORMAT'): # How should results look?
                    opts = opts-1   # this is not an option for indexing purposes
                    level = upper(opt[8:])
                    if level in ('ALL','EVERYTHING','VERBOSE', 'MAX'):
                        ndx.quiet = 0
                    elif level in ('RATINGS','SCORES','HIGH'):
                        ndx.quiet = 3
                    elif level in ('FILENAMES','NAMES','FILES','MID'):
                        ndx.quiet = 5
                    elif level in ('SUMMARY','MIN'):
                        ndx.quiet = 9
                elif upper(opt[1:7]) == 'FILTER':           # Regex filter output
                    opts = opts-1   # this is not an option for indexing purposes
                    ndx.filter = opt[8:]
                elif opt[1:] in string.digits:
                    opts = opts-1
                    ndx.quiet = eval(opt[1])
            else:
                search_words.append(opt)                    # Search words

    if opts > 0:
        ndx.add_files(dir=dir)
        ndx.save_index()
    if search_words:
        ndx.find(search_words, print_report=1)
    if not opts and not search_words:
        sys.stderr.write("Perhaps you would like to use the --help option?\n")
    else:
        sys.stderr.write('Processed in %.3f seconds (%s)\n'
                          % (time.time()-start, ndx.whoami()))

