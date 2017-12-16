

class vec3d {
public:
	float v[3];

	vec3d(float v0=0, float v1=0, float v2=0){
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}
	~vec3d(){};

	void set(float v0, float v1, float v2){
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}

	vec3d operator+(float a){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] + a;

		return tmp;
	}

	vec3d operator-(float a){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] - a;

		return tmp;
	}

	vec3d operator*(float a){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] * a;

		return tmp;
	}

	vec3d operator/(float a){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] / a;

		return tmp;
	}

	vec3d operator+(vec3d vec){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] + vec.v[i];
		
		return tmp;
	}

	vec3d operator-(vec3d vec){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] - vec.v[i];

		return tmp;
	}

	vec3d operator*(vec3d vec){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] * vec.v[i];

		return tmp;
	}

	vec3d operator/(vec3d vec){
		vec3d tmp;
		for (int i = 0; i < 3; i++)
			tmp.v[i] = this->v[i] / vec.v[i];

		return tmp;
	}
};