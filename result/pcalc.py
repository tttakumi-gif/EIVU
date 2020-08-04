import re
import codecs
from sys import argv

if len(argv) <= 1:
	exit()

rfloat = r".*?(\d+(?:\.\d+)?).*?"
rint = r".*?(\d+).*?"

types = ("task-clock", "context-switches", "cpu-migrations", "page-faults")
patterns = (r"{}{}".format(rfloat, types[0]), r"{}{}".format(rint, types[1]), r"{}{}".format(rint, types[2]), r"{}{}".format(rint, types[3]))

otypes = ("cycles  ", "stalled-cycles-frontend", "stalled-cycles-backend", "instructions", "cache-references", "cache-misses", "branches", "branch-misses")
opatterns = [r"{}{}".format(rint, otypes[i]) for i in range(len(otypes))]
count = 0
count_result = 0
msum = 0.0
mave = 0.0
val = 0.0

array_ave = [0 for i in range(len(types) + len(otypes))]

with codecs.open(argv[1], "r", "utf-8", "ignore") as f:
	for row in f:
		if count < len(types):
			for pattern in patterns:
				result = re.findall(pattern, row)
				if result:
					#print("{}: {}".format(types[count], result[0]))
					try:
						#print(result)
						array_ave[count] += int(result[0])
					except ValueError:
						array_ave[count] += float(result[0])
					count += 1
					break

		else:
			for pattern in opatterns:
				result = re.match(pattern, row)
				if result:
					spl = row.split("s")
					sval = spl[0].split(",")

					scount = 1
					num = 0
					for val in reversed(sval):
						rval = re.findall(rint, val)
						if rval:
							num += (int(rval[0]) * scount)
							scount *= 1000
					#print("{}: {:,}".format(otypes[count - len(types)], num))
					array_ave[count] += num
					count += 1
					break

		if len(types) + len(otypes) <= count:
			count = 0
			count_result += 1

filepath = "."
if(2 < len(argv)):
	filepath = argv[2]

print(count_result)
print(array_ave[0])
with open("{}/result_perf".format(filepath), "w") as fi:
	count = 0
	for a in array_ave:
		if(count < len(types)):
			fi.write("{}: {:,}\n".format(types[count], a / count_result))
		else:
			fi.write("{}: {:,}\n".format(otypes[count - len(types)], a / count_result))
		count += 1
"""
				count += 1
				sec = row.replace("result: ", "")
				msum += float(sec.replace("sec", ""))

		mave = msum / count

	with codecs.open("log", "r", "utf-8", "ignore") as f:
		for row in f:
			result = re.match(pattern, row)
			if result:
				sec = row.replace("result: ", "")
				val += ((float(sec.replace("sec", "")) - mave) ** 2)


	with open("log", "w") as fi:
		fi.write("ave: {:.3f}sec\n".format(mave))
		fi.write("var: {:.3f}\n".format(val / count))
"""
