filename = "./ratio.txt"

miss = 0
hit = 0
with open(filename) as f:
	l_strip = [s.strip() for s in f.readlines()]
	miss += int(l_strip[1].replace(',', ''))
	hit += int(l_strip[3].replace(',', ''))
	hit += int(l_strip[5].replace(',', ''))

print("miss: {}".format(miss))
print("hit: {}".format(hit))
print("miss ratio: {}%".format(miss / hit * 100))
