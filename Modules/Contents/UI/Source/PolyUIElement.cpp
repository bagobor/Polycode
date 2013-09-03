/*
 Copyright (C) 2012 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "PolyUIElement.h"
#include "PolyRenderer.h"
#include "PolyCoreServices.h"
#include "PolyTexture.h"

using namespace Polycode;

UIElement *UIElement::globalFocusedChild = NULL;

UIRect::UIRect(String fileName) : UIElement() {
	texture = NULL;
	loadTexture(fileName);
	if(texture) {	
		initRect(texture->getWidth(), texture->getHeight());
		imageWidth = texture->getWidth();
		imageHeight = texture->getHeight();	
	} else {
		initRect(1,  1);
		imageWidth = 0;
		imageHeight = 0;
	}
}

UIRect::UIRect(Number width, Number height) : UIElement() {
	texture = NULL;
	initRect(width, height);
	imageWidth = 0;
	imageHeight = 0;
}

void UIRect::setImageCoordinates(Number x, Number y, Number width, Number height) {
	Vertex *vertex;
	Number pixelSizeX = 1/imageWidth;
	Number pixelSizeY = 1/imageHeight;

	setWidth(width);
	setHeight(height);

	Number whalf = width/2.0f;
	Number hhalf = height/2.0f;
		
	Number xFloat = x * pixelSizeX;
	Number yFloat = (y * pixelSizeY);
	Number wFloat = width * pixelSizeX;
	Number hFloat = height * pixelSizeY;

	Polygon *imagePolygon = rectMesh->getPolygon(0);	
	vertex = imagePolygon->getVertex(0);
	vertex->set(-whalf,-hhalf,0);
	vertex->setTexCoord(xFloat, yFloat + hFloat);

	vertex = imagePolygon->getVertex(1);
	vertex->set(-whalf+width,-hhalf,0);
	vertex->setTexCoord(xFloat + wFloat, yFloat + hFloat);

	vertex = imagePolygon->getVertex(2);
	vertex->set(-whalf+width,-hhalf+height,0);
	vertex->setTexCoord(xFloat + wFloat, yFloat);

	vertex = imagePolygon->getVertex(3);	
	vertex->set(-whalf,-hhalf+height,0);	
	vertex->setTexCoord(xFloat, yFloat);

	rectMesh->arrayDirtyMap[RenderDataArray::VERTEX_DATA_ARRAY] = true;
	rectMesh->arrayDirtyMap[RenderDataArray::TEXCOORD_DATA_ARRAY] = true;	
	rebuildTransformMatrix();
	matrixDirty = true;
}

Number UIRect::getImageWidth() const {
	return imageWidth;
}

Number UIRect::getImageHeight() const {
	return imageHeight;	
}

void UIRect::initRect(Number width, Number height) {
	rectMesh = new Mesh(Mesh::QUAD_MESH);
	processInputEvents = true;

	setAnchorPoint(-1.0, -1.0, 0.0);
	setWidth(width);	
	setHeight(height);
	
	Number whalf = width/2.0f;
	Number hhalf = height/2.0f;
						
	Polygon *poly = new Polygon();
	poly->addVertex(-whalf,-hhalf,0,0,0);
	poly->addVertex(-whalf+width,-hhalf,0, 1, 0);
	poly->addVertex(-whalf+width,-hhalf+height,0, 1, 1);
	poly->addVertex(-whalf,-hhalf+height,0,0,1);
	rectMesh->addPolygon(poly);
}

UIRect::~UIRect() {
	delete rectMesh;
}

void UIRect::loadTexture(String fileName) {
	MaterialManager *materialManager = CoreServices::getInstance()->getMaterialManager();
	texture = materialManager->createTextureFromFile(fileName, materialManager->clampDefault, false);
}

void UIRect::setTexture(Texture *texture) {
	this->texture = texture;
}	

Texture *UIRect::getTexture() {
	return texture;
}

void UIRect::Render() {
	renderer->clearShader();
	renderer->setTexture(texture);
	renderer->pushDataArrayForMesh(rectMesh, RenderDataArray::VERTEX_DATA_ARRAY);
	renderer->pushDataArrayForMesh(rectMesh, RenderDataArray::TEXCOORD_DATA_ARRAY);	
	renderer->drawArrays(Mesh::QUAD_MESH);
}

void UIRect::Resize(Number width, Number height) {

	setWidth(width);	
	setHeight(height);

	Number whalf = width/2.0f;
	Number hhalf = height/2.0f;
	Polygon *polygon;
	Vertex *vertex;

	polygon = rectMesh->getPolygon(0);	
	vertex = polygon->getVertex(0);
	vertex->set(-whalf,-hhalf,0);			
	vertex = polygon->getVertex(1);
	vertex->set(-whalf+width,-hhalf,0);			
	vertex = polygon->getVertex(2);
	vertex->set(-whalf+width,-hhalf+height,0);			
	vertex = polygon->getVertex(3);	
	vertex->set(-whalf,-hhalf+height,0);				
	rectMesh->arrayDirtyMap[RenderDataArray::VERTEX_DATA_ARRAY] = true;	
}

UIImage::UIImage(String imagePath) : UIElement() {
	image = new SceneImage(imagePath);
	image->setAnchorPoint(-1.0, -1.0, 0.0);
	image->depthTest = false;
	image->depthWrite = false;		
	addChild(image);
	setWidth(image->bBox.x);
	setHeight(image->bBox.y);	
}

SceneImage *UIImage::getImage() {
	return image;
}

UIElement::UIElement() : Entity() {
	setAnchorPoint(-1.0, -1.0, 0.0);
	processInputEvents = true;
	depthTest = false;	
	hasFocus = false;
	focusable = false;
	focusParent = NULL;
	hasDragLimits = false;
	dragged = false;
	depthTest = false;
	depthWrite = false;			
}

UIElement::UIElement(Number width, Number height) : Entity() {
	setAnchorPoint(-1.0, -1.0, 0.0);
	processInputEvents = true;
	focusParent = NULL;
	hasFocus = false;
	hasDragLimits = false;	
	dragged = false;	
	depthTest = false;
	depthWrite = false;		
	setWidth(width);
	setHeight(height);
}

void UIElement::addChild(Entity *child) {
	UIElement* uiChild = dynamic_cast<UIElement*>(child);
	if(uiChild) {
		addFocusChild(uiChild);
	}
	Entity::addChild(child);
}

void UIElement::setDragLimits(Rectangle rect) {
	dragLimits.x = rect.x;
	dragLimits.y = rect.y;
	dragLimits.w = rect.w;
	dragLimits.h = rect.h;
	hasDragLimits = true;
}

void UIElement::clearDragLimits() {
	hasDragLimits = false;
}

bool UIElement::isDragged() {
	return dragged;
}

void UIElement::startDrag(Number xOffset, Number yOffset) {
	dragged = true;
	dragOffsetX = xOffset;
	dragOffsetY = yOffset;
}

void UIElement::stopDrag() {
	dragged = false;
}

bool UIElement::isFocusable() {
	return focusable;
}

MouseEventResult UIElement::onMouseMove(const Ray &ray, int timestamp) {
	if(dragged) {	
		Vector3 localCoordinate = Vector3(ray.origin.x, ray.origin.y,0);

		if(parentEntity) {
			Matrix4 inverse = parentEntity->getConcatenatedMatrix().Inverse();
			localCoordinate = inverse * localCoordinate;		
		}
		setPosition(localCoordinate.x-dragOffsetX,-localCoordinate.y-dragOffsetY);
		if(hasDragLimits) {
			if(position.x < dragLimits.x)
				position.x = dragLimits.x;
			if(position.x > dragLimits.x + dragLimits.w)
				position.x = dragLimits.x + dragLimits.w;
			if(position.y < dragLimits.y)
				position.y = dragLimits.y;
			if(position.y > dragLimits.y + dragLimits.h)
				position.y = dragLimits.y + dragLimits.h;
		}		
		
	}
	return Entity::onMouseMove(ray, timestamp);
}

UIElement::~UIElement() {
	if(UIElement::globalFocusedChild == this) {
		UIElement::globalFocusedChild = NULL;
	}
}

void UIElement::focusNextChild() {

	int j = 0;
	bool hasFocusedChild = false;
	if(UIElement::globalFocusedChild) {
		for(int i=0; i < focusChildren.size(); i++) {
			if(focusChildren[i] == UIElement::globalFocusedChild) {
				j = i;
				hasFocusedChild = true;
			}
		}
	}

	if(!hasFocusedChild)
		return;

	for(int i=0; i < focusChildren.size(); i++) {
		if(focusChildren[j]->isFocusable() && focusChildren[j] != UIElement::globalFocusedChild) {
			focusChild(focusChildren[j]);
			return;
		}

		j++;
		if(j == focusChildren.size())
			j = 0;
	}
}

void UIElement::focusSelf() {
	if(UIElement::globalFocusedChild) {
		UIElement::globalFocusedChild->onLoseFocus();
		UIElement::globalFocusedChild->hasFocus = false;
	}

	UIElement::globalFocusedChild = this;
	
	onGainFocus();
	hasFocus = true;
}

void UIElement::focusChild(UIElement *child) {

	if(UIElement::globalFocusedChild) {
		UIElement::globalFocusedChild->onLoseFocus();
		UIElement::globalFocusedChild->hasFocus = false;
	}

	UIElement::globalFocusedChild = child;
	
	if(child) {
		UIElement::globalFocusedChild->onGainFocus();
		UIElement::globalFocusedChild->hasFocus = true;
	}	
}

void UIElement::addFocusChild(UIElement *element) {
	element->setFocusParent(element);
	focusChildren.push_back(element);
}

void UIElement::setFocusParent(UIElement *element) {
	focusParent = element;	
}

void UIElement::Resize(Number width, Number height) {
	setWidth(width);
	setHeight(height);
	dirtyMatrix(true);	
}
