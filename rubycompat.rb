class Object
	def stdout
		STDOUT
	end
	def stdin
		STDIN
	end
	def write(str)
		print(str)
	end
end

class Array
	def to_s
		inspect
	end
end

class String
	alias_method :_plus_, :+
	def -(arg)
		self.delete(arg)
	end
	def +(arg)
		self._plus_(arg.to_s)
	end
end
