from fontawesome import icons

ls = [
    'toggle-on',
    'toggle-off',
    'tachometer-alt',
    'biking'
]

ld = {}
with open('fa_list.txt', 'w') as f:
    for l in ls:
        ld[l] = icons[l]
        f.write(icons[l])
        print(icons[l], l)
print(ld)
