# BlenderBIM Add-on - OpenBIM Blender Add-on
# Copyright (C) 2020, 2021 Dion Moult <dion@thinkmoult.com>
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

import bpy
import blenderbim.bim.module.drawing.decoration as decoration
from bpy.app.handlers import persistent


@persistent
def toggleDecorationsOnLoad(*args):
    toggle = bpy.context.scene.DocProperties.should_draw_decorations
    if toggle:
        decoration.DecorationsHandler.install(bpy.context)
    else:
        decoration.DecorationsHandler.uninstall()


@persistent
def depsgraph_update_pre_handler(scene):
    set_active_camera_resolution(scene)


def set_active_camera_resolution(scene):
    if not scene.camera or "/" not in scene.camera.name or not scene.DocProperties.drawings:
        return
    if (
        scene.render.resolution_x != scene.camera.data.BIMCameraProperties.raster_x
        or scene.render.resolution_y != scene.camera.data.BIMCameraProperties.raster_y
    ):
        scene.render.resolution_x = scene.camera.data.BIMCameraProperties.raster_x
        scene.render.resolution_y = scene.camera.data.BIMCameraProperties.raster_y
    current_drawing = scene.DocProperties.drawings[scene.DocProperties.current_drawing_index]
    if scene.camera != current_drawing.camera:
        scene.DocProperties.current_drawing_index = scene.DocProperties.drawings.find(scene.camera.name.split("/")[1])
        bpy.ops.bim.activate_view(drawing_index=scene.DocProperties.current_drawing_index)
