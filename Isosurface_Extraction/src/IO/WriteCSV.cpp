#include <IO/WriteCSV.h>
#include <iostream>

template <class T>
WriteCSV<T>::WriteCSV(std::string outputFile) {
	p_os = std::ofstream(outputFile);
	p_outFile = outputFile;
}

/// <summary>
/// The map should be keyed by the CSV headers, 
/// The values should be the columns for that go with those headers
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="values"></param>
/// <returns></returns>
template<class T>
bool WriteCSV<T>::write(const std::unordered_map<std::string, std::vector<T>>& values) {
	if (values.empty()) {
		p_os.close();
		std::cerr << "Warning: WriteCSV, CSV is empty!" << std::endl;
		return true;
	}

	std::vector<const std::vector<T>*> columns(values.size());

	size_t i = 0;
	for (auto& [header, column_values] : values) {
		p_os << header << ",";
		columns[i++] = &column_values;
	}
	p_os << "\n";

	for (size_t row = 0; row < columns[0]->size(); ++row) {
		for (size_t col = 0; col < columns.size(); ++col)
			p_os << (*columns[col])[row] << ",";
		p_os << "\n";
	}

	p_os.close();

	std::cout << "WriteCSV: CSV file " << p_outFile << " written successfully." << std::endl;

	return true;
}


template class WriteCSV<int>;
template class WriteCSV<float>;
template class WriteCSV<double>;
template class WriteCSV<std::string>;