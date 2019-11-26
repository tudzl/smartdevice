#https://grokbase.com/t/python/python-list/003dxmrfhs/itoa
data=[101,65,42]
s= chr(101)
s= 'e'
s= "".join(map(chr,data))
'eA*'

def itoa (n, base = 2):
	if type (n) != types.IntType:
	raise TypeError, 'First arg should be an integer'
	if (type (base) != types.IntType) or (base <= 1):
	raise TypeError, 'Second arg should be an integer greater than 1'
	output = []
	pos_n = abs (n)
	while pos_n:
	lowest_digit = pos_n % base
	output.append (str (lowest_digit))
	pos_n = (pos_n - lowest_digit) / base
	output.reverse ()
	if n < 0:
	output.insert (0, '-')
	return string.join (output, '')

	二进制  八进制  十六进制转换
16->2         bin(int(x,16))
10->8         oct(int(x,10))
2->16        hex(int(x,2))