import re
import codecs

pattern = ".*?(\d+)sec"
count = 0
msum = 0.0

with codecs.open("log", "r", "utf-8", "ignore") as f:
	for row in f:
		result = re.match(pattern, row)
		if result:
			count += 1
			msum += float(row.replace("sec", ""))

with open("log", "w") as fi:
	fi.write("{:.3f}sec\n".format(msum / count))
