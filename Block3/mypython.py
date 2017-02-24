# Sammy Petinichi
# CS344 - Operating Systems
# Python Program

#!/usr/bin/python

import random
import string

#declare file names
file1 = "file1"
file2 = "file2"
file3 = "file3"

#open files for writing
f1 = open(file1,"w+")
f2 = open(file2,"w+")
f3 = open(file3,"w+")

#write 10 random letters to files
for i in range(10):
  f1.write(random.choice(string.ascii_lowercase))
  f2.write(random.choice(string.ascii_lowercase))
  f3.write(random.choice(string.ascii_lowercase))

#add newlines
f1.write("\n")
f2.write("\n")
f3.write("\n")

#close files
f1.close
f2.close
f3.close

#open files for reading
f1 = open(file1,"r")
f2 = open(file2,"r")
f3 = open(file3,"r")

#print contents of files
print(f1.read(),end="")
print(f2.read(),end="")
print(f3.read(),end="")

#close files
f1.close
f2.close
f3.close

#generate two random numbers from 1-42
num1 = random.randint(1,42)
num2 = random.randint(1,42)
#print the two numbers
print(num1)
print(num2)
#print the product of the two numbers
print(num1*num2)
