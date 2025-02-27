#include "../ifcgeom_schema_agnostic/IfcGeomRenderStyles.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <map>
#include <mutex>

namespace pt = boost::property_tree;

static std::map<std::string, std::shared_ptr<IfcGeom::SurfaceStyle>> default_materials;
static std::shared_ptr<IfcGeom::SurfaceStyle> default_material;
static bool default_materials_initialized = false;

void InitDefaultMaterials() {
	default_materials.insert(std::make_pair("IfcSite", std::make_shared<IfcGeom::SurfaceStyle>("IfcSite")));
	default_materials["IfcSite"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.75, 0.8, 0.65));

	default_materials.insert(std::make_pair("IfcSlab", std::make_shared<IfcGeom::SurfaceStyle>("IfcSlab")));
	default_materials["IfcSlab"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.4, 0.4, 0.4));

	default_materials.insert(std::make_pair("IfcWallStandardCase", std::make_shared<IfcGeom::SurfaceStyle>("IfcWallStandardCase")));
	default_materials["IfcWallStandardCase"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.9, 0.9, 0.9));

	default_materials.insert(std::make_pair("IfcWall", std::make_shared<IfcGeom::SurfaceStyle>("IfcWall")));
	default_materials["IfcWall"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.9, 0.9, 0.9));

	default_materials.insert(std::make_pair("IfcWindow", std::make_shared<IfcGeom::SurfaceStyle>("IfcWindow")));
	default_materials["IfcWindow"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.75, 0.8, 0.75));
	default_materials["IfcWindow"]->Transparency().reset(0.3);

	default_materials.insert(std::make_pair("IfcDoor", std::make_shared<IfcGeom::SurfaceStyle>("IfcDoor")));
	default_materials["IfcDoor"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.55, 0.3, 0.15));

	default_materials.insert(std::make_pair("IfcBeam", std::make_shared<IfcGeom::SurfaceStyle>("IfcBeam")));
	default_materials["IfcBeam"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.75, 0.7, 0.7));

	default_materials.insert(std::make_pair("IfcRailing", std::make_shared<IfcGeom::SurfaceStyle>("IfcRailing")));
	default_materials["IfcRailing"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.65, 0.6, 0.6));

	default_materials.insert(std::make_pair("IfcMember", std::make_shared<IfcGeom::SurfaceStyle>("IfcMember")));
	default_materials["IfcMember"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.65, 0.6, 0.6));

	default_materials.insert(std::make_pair("IfcPlate", std::make_shared<IfcGeom::SurfaceStyle>("IfcPlate")));
	default_materials["IfcPlate"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.8, 0.8, 0.8));

	default_materials.insert(std::make_pair("IfcSpace", std::make_shared<IfcGeom::SurfaceStyle>("IfcSpace")));
	default_materials["IfcWindow"]->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.65, 0.75, 0.8));
	default_materials["IfcWindow"]->Transparency().reset(0.8);

	default_material = std::make_shared<IfcGeom::SurfaceStyle>("DefaultMaterial");
	default_material->Diffuse().reset(IfcGeom::SurfaceStyle::ColorComponent(0.7, 0.7, 0.7));

	default_materials_initialized = true;
}

boost::optional<IfcGeom::SurfaceStyle::ColorComponent> read_colour_component(const boost::optional<pt::ptree&> list) {
	if (!list) {
		return boost::none;
	}
	double rgb[3];
	int i = 0;
	for (pt::ptree::value_type &colour : list.get()) {
		if (3 <= i) {
			throw std::runtime_error("rgb array over 3 elements large");
		}
		rgb[i] = colour.second.get_value<double>();
		i++;
	}
	if (i != 3) {
		throw std::runtime_error("rgb array less than 3 elements large (was " + std::to_string(i) + ")");
	}
	return IfcGeom::SurfaceStyle::ColorComponent(rgb[0], rgb[1], rgb[2]);
}

void IfcGeom::set_default_style_file(const std::string& json_file) {
	if (!default_materials_initialized) InitDefaultMaterials();
	default_materials.clear();

	// @todo this will probably need to be updated for UTF-8 paths on Windows
	pt::ptree root;
	pt::read_json(json_file, root);

	for (pt::ptree::value_type &material_pair : root) {
		std::string name = material_pair.first;
		default_materials.insert(std::make_pair(name, std::make_shared<IfcGeom::SurfaceStyle>(name)));

		pt::ptree material = material_pair.second;
		boost::optional<pt::ptree&> diffuse = material.get_child_optional("diffuse");
		default_materials[name]->Diffuse() = read_colour_component(diffuse);

		boost::optional<pt::ptree&> specular = material.get_child_optional("specular");
		default_materials[name]->Specular() = read_colour_component(specular);

		if (material.get_child_optional("specular-roughness")) {
			default_materials[name]->Specularity().reset(1.0 / material.get<double>("specular-roughness"));
		}
		if (material.get_child_optional("transparency")) {
			default_materials[name]->Transparency() = material.get<double>("transparency");
		}
	}

	// Is "*" present? If yes, remove it and make it the default style.
	auto it = default_materials.find("*");
	if (it != default_materials.end()) {
		default_material = it->second;
		default_materials.erase(it);
	}
}

std::shared_ptr<const IfcGeom::SurfaceStyle> IfcGeom::get_default_style(const std::string& s) {
	static std::mutex m;
	std::lock_guard<std::mutex> lk(m);

	if (!default_materials_initialized) InitDefaultMaterials();
	auto it = default_materials.find(s);
	if (it == default_materials.end()) {
		default_materials.insert(std::make_pair(s, default_material));
		it = default_materials.find(s);
	}
	return it->second;
}

IfcGeom::SurfaceStyle& IfcGeom::update_default_style(const std::string& s) {
	if (!default_materials_initialized) InitDefaultMaterials();
	auto it = default_materials.find(s);
	if (it == default_materials.end()) {
		throw std::runtime_error("No style registered for " + s);
	}
	return *it->second;
}
