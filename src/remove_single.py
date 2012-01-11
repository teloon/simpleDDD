import sys

inn = open("sort-metadata", 'r+');
out = open("metadata", 'w');

first = inn.readline()
second = inn.readline()
if first[0:16] == second[0:16]:
	out.write(first)
for third in inn:
	if second[0:16] == first[0:16] or second[0:16] == third[0:16]:
		out.write(second)
	first = second
	second = third
if first[0:16] == second[0:16]:
	out.write(second)

def hello():
	print "hello!"
