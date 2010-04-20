from gnosis.magic import DTDGenerator, \
                         import_with_metaclass, from_import

diss = import_with_metaclass('dissertation', DTDGenerator)
from_import(diss,'**')

if __name__=='__main__':
    from gnosis.xml.validity import PCDATA
    class section(Seq): _order = (dedication, chapter)
    sect = LiftSeq(section,('To Mom',
                            ('It starts','When it began...')))

    print sect
    print sect.with_internal_subset()

    print eval(explicit_section).with_internal_subset()
    print eval(lifting_section).with_internal_subset()

    "Miscellaneous other typical uses"
    p = paragraph(['spam and eggs', figure(), 'toast'])
    ps_ch1 = _paras([ p, paragraph('this and that'), "something else",
                      paragraph(_mixedpara(figure())),
                      paragraph(['stuff', table(), PCDATA('still more')])
                     ])
    chap1 = chapter((title('My chapter'), ps_ch1))
    dedict = _dedi(dedication("To Mom"))
    diss = LiftSeq( dissertation, ([], _chaps(chap1), _apps()) )
    ps_ch1 += [ "friend, romans, countrymen",
                paragraph([figure(), table(), figure()]) ]
    ps_ch1.append("whatever")
    ps_ch1 += "Just words"
    del ps_ch1[-2]
    print diss.with_internal_subset()


