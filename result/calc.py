import re
import codecs

pattern = "result: .*?(\d+)sec"
count = 0
msum = 0.0
mave = 0.0
val = 0.0

with codecs.open("log", "r", "utf-8", "ignore") as f:
	for row in f:
		result = re.match(pattern, row)
		if result:
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
