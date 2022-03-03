#ifndef UTIL_CSV_WRITER
#define UTIL_CSV_WRITER

#include <fstream>
#include <string>
#include <sstream>

class CsvWriter {
	std::ofstream csvFile;
	bool newLine = true;
	
	template<typename V>
	void writeValue(V &&v) {
		std::stringstream strstr;
		strstr << v;
		std::string str = strstr.str();
		bool needsQuote = false;
		for (unsigned i = 0; i < str.size(); ++i) {
			if (str[i] == ',') {
				needsQuote = true;
				break;
			}
		}
		if (needsQuote) {
			csvFile << "\"";
			for (unsigned i = 0; i < str.size(); ++i) {
				if (str[i] == '"') {
					csvFile << '"';
				}
				csvFile << str[i];
			}
			csvFile << "\"";
		} else {
			csvFile << v;
		}
	}
	
	void writeValue(const double &v) {
		csvFile << v;
	}
	void writeValue(const float &v) {
		csvFile << v;
	}
public:
	CsvWriter(std::string name) : csvFile(name + ".csv") {}
	
	CsvWriter & write() {
		return *this;
	}
	template<class First, class... Args>
	CsvWriter & write(const First &v, Args ...args) {
		if (!newLine) csvFile << ",";
		newLine = false;
		writeValue(v);
		return write(args...);
	}
	template<typename... T>
	CsvWriter & line(T... t) {
		write(t...);
		csvFile << "\n";
		newLine = true;
		return *this;
	}
};

#endif
