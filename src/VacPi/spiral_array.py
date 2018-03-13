from math import sqrt


def spiral_factor(x):
    return 1-1/sqrt(1+(91/90000)*x**2)


file = open("spiral_array.h", "w")

print("const float spiral_factor[] = { 0.0", end='', file=file)
for x in range(1, 101):
    print(",", end='\n', file=file)
    print(spiral_factor(x), end='', file=file)
print("};", end='\n', file=file)
