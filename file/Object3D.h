#pragma once
#include "shprogram.h"
#include <glm/gtc/matrix_transform.hpp>


class Object3D
{
protected:
	glm::mat4 model;

public:
	Object3D();
	virtual ~Object3D();

	virtual void Draw(ShaderProgram shader) = 0;

	const glm::mat4& getModelMatrix() const {
		return model;
	}

	void setToIdentity() {
		model = glm::mat4(1.0f);
	}

	glm::vec3 getOrigin() {
		return  glm::vec3(model * glm::vec4(0.0f));
	}

	void translate(const glm::vec3& v) {
		model = glm::translate(model, v);
	}

	void rotate(const glm::vec3& axis, float angle) {
		model = glm::rotate(model, angle, axis);
	}

	void scale(const glm::vec3& scale) {
		model = glm::scale(model, scale);
	}

	void transform(const glm::mat4& transformation)
	{
		model = transformation * model;
	}

	void rotateAroundPoint(const glm::vec3& axis, float angle, const glm::vec3& worldPoint);
};

