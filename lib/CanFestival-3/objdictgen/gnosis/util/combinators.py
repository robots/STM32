from operator import mul, add, truth

apply_each = lambda fns, args=[]: map(apply, fns, [args]*len(fns))
bools = lambda lst: map(truth, lst)
bool_each = lambda fns, args=[]: bools(apply_each(fns, args))
conjoin = lambda fns, args=[]: reduce(mul, bool_each(fns, args))
all = lambda fns: lambda arg, fns=fns: conjoin(fns, (arg,))
both = lambda f,g: all((f,g))
all3 = lambda f,g,h: all((f,g,h))
and_ = lambda f,g: lambda x, f=f, g=g: f(x) and g(x)
disjoin = lambda fns, args=[]: reduce(add, bool_each(fns, args))
some = lambda fns: lambda arg, fns=fns: disjoin(fns, (arg,))
either = lambda f,g: some((f,g))
anyof3 = lambda f,g,h: some((f,g,h))
or_ = lambda f,g: lambda x, f=f, g=g: f(x) or g(x)
compose = lambda f,g: lambda x, f=f, g=g: f(g(x))
compose3 = lambda f,g,h: lambda x, f=f, g=g, h=h: f(g(h(x)))
ident = lambda x: x
not_ = lambda f: lambda x, f=f: not f(x)

def shortcut_all(*fns):
    accum = fns[0]
    for fn in fns[1:]:
        accum = and_(accum, fn)
    return accum
lazy_all = shortcut_all

def shortcut_any(*fns):
    accum = fns[0]
    for fn in fns[1:]:
        accum = or_(accum, fn)
    return accum
lazy_any = shortcut_any
