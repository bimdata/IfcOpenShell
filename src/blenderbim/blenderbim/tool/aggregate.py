# BlenderBIM Add-on - OpenBIM Blender Add-on
# Copyright (C) 2021 Dion Moult <dion@thinkmoult.com>
#
# This file is part of BlenderBIM Add-on.
#
# BlenderBIM Add-on is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# BlenderBIM Add-on is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with BlenderBIM Add-on.  If not, see <http://www.gnu.org/licenses/>.

import blenderbim.core.tool
import blenderbim.tool as tool


class Aggregate(blenderbim.core.tool.Aggregate):
    @classmethod
    def enable_editing(cls, obj):
        obj.BIMObjectAggregateProperties.is_editing = True

    @classmethod
    def disable_editing(cls, obj):
        obj.BIMObjectAggregateProperties.is_editing = False

    @classmethod
    def can_aggregate(cls, relating_obj, related_obj):
        relating_object = tool.Ifc.get_entity(relating_obj)
        related_object = tool.Ifc.get_entity(related_obj)
        if not relating_object or not related_object:
            return False
        if relating_object.is_a("IfcElement") and related_object.is_a("IfcElement"):
            if relating_obj.data:
                return False
            return True
        if tool.Ifc.get_schema() == "IFC2X3":
            if relating_object.is_a("IfcSpatialStructureElement") and related_object.is_a("IfcSpatialStructureElement"):
                return True
            if relating_object.is_a("IfcProject") and related_object.is_a("IfcSpatialStructureElement"):
                return True
        else:
            if relating_object.is_a("IfcSpatialElement") and related_object.is_a("IfcSpatialElement"):
                return True
            if relating_object.is_a("IfcProject") and related_object.is_a("IfcSpatialElement"):
                return True
        return False
