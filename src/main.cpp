#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using Matrix = std::array<float, 3 * 3>;

sf::Vector2f matrixMul(const Matrix &matrix, const sf::Vector3f &v);

const Matrix identity2d = {
    1, 0, 0, 0, 1, 0, 0, 0, 0,
};

struct Face : public sf::Shape {
	size_t getPointCount() const { return mVertecies.size(); }

	sf::Vector2f getPoint(size_t index) const {
		return matrixMul(identity2d, mVertecies[index]);
	}

	void addPoint(const sf::Vector3f &vertex) {
		mVertecies.emplace_back(vertex);
		update();
	}

	void setNormal(sf::Vector3f normal) { mNormal = normal; }
	sf::Vector3f getNormal() const { return mNormal; }

  private:
	sf::Vector3f mNormal;
	std::vector<sf::Vector3f> mVertecies;
};

template <typename T>
std::ostream &operator<<(std::ostream &stream, const sf::Vector3<T> &v) {
	stream << v.x << ", " << v.y << ", " << v.z;
	return stream;
}

std::vector<std::string> split(const std::string &str, const char delimiter) {
	std::vector<std::string> strings;
	std::istringstream sstream(str);
	std::string token;
	while (std::getline(sstream, token, delimiter)) {
		strings.emplace_back(std::move(token));
	}

	return strings;
}

enum Element {
	vertex = 0,
	vertexNormal,
	textureVertex,
	face,
};

uint getCount(const std::vector<std::string> &lines, const Element &element) {
	uint count = 0;
	char begin[2] = " ";
	for (auto &line : lines) {
		switch (element) {
		case vertex:
			begin[0] = 'v';
			begin[1] = ' ';
			break;
		case vertexNormal:
			begin[0] = 'v';
			begin[1] = 'n';
			break;
		case textureVertex:
			begin[0] = 'v';
			begin[1] = 't';
			break;
		case face:
			begin[0] = 'f';
			begin[1] = ' ';
			break;
		default:
			break;
		}
		count += static_cast<uint>(line.starts_with(begin));
	}
	return count;
}

std::vector<Face> parseFile(std::ifstream &file) {
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) {
		lines.emplace_back(line);
	}

	std::vector<sf::Vector3f> vertecies;
	vertecies.reserve(getCount(lines, Element::vertex));
	std::vector<sf::Vector3f> normals;
	normals.reserve(getCount(lines, Element::vertexNormal));
	std::vector<Face> faces;
	faces.reserve(getCount(lines, Element::face));

	for (const auto &line : lines) {
		if (line.starts_with("v ")) {
			sf::Vector3f vertex;
			std::stringstream s(line);
			s.seekg(1);
			s >> vertex.x >> vertex.y >> vertex.z;
			vertecies.emplace_back(vertex);
		}

		if (line.starts_with("vn ")) {
			auto elements = split(line, ' ');
			normals.emplace_back(std::stof(elements[1]), std::stof(elements[2]),
			                     std::stof(elements[3]));
		}

		if (line.starts_with("f ")) {
			Face face;
			auto strings = split(line, ' ');
			for (auto it = strings.begin() + 1; it != strings.end(); ++it) {
				auto &string = *it;
				auto element = split(string, '/');
				face.addPoint(vertecies[std::stoi(element[0]) - 1]);
				face.setNormal(normals[std::stoi(element[2]) - 1]);
			}
			faces.emplace_back(face);
		}
	}

	return faces;
}

int main() {
	std::ifstream file("media/cube.obj");
	if (!file.is_open()) {
		std::cerr << "Couldn't open file\n";
	}

	auto faces = parseFile(file);
	sf::Color color(0, 0, 100);
	for (auto &face : faces) {
		face.setFillColor(color);
		if (color.b + 50 >= 0xff) {
			color.b += 50;
		} else {
			color.g += 75;
		}
		face.setScale({100, 100});
		face.setPosition({300, 300});
	}

	sf::RenderWindow window(sf::VideoMode({800, 800}), "Cubes!");

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		window.clear();
		for (const auto &face : faces) {
			sf::Vector2f angle(face.getNormal().x, face.getNormal().z);
			if (angle.angle().wrapSigned().asDegrees() < 0)
				window.draw(face);
		}
		window.display();
	}
}

sf::Vector2f matrixMul(const Matrix &matrix, const sf::Vector3f &v) {
	float varr[3] = {v.x, v.y, v.z};

	float temp[3];
	temp[0] = 0;
	temp[1] = 0;
	temp[2] = 0;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			temp[i] += varr[j] * matrix[3 * i + j];
		}
	}
	sf::Vector2f out = *(sf::Vector2f *)temp;
	return out;
}
