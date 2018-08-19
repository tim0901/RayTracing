#pragma once

#include "ray.h"
#include "hitable.h"

float schlick(float cosine, float ref_idx) {
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0)*pow((1 - cosine), 5);
}

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
	vec3 uv = unit_vector(v);
	float dt = dot(uv, n);
	float discriminant = 1.0 - ni_over_nt * ni_over_nt*(1 - dt * dt);
	if (discriminant > 0) {
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
		return false;
}

vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n)*n;
}

vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0*vec3((rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0))) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1.0);
	return p;
}

//Tells us how rays interact with the surface
class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;

};

//Defines properties of a lambertian material
class lambertian : public material {
public:
	lambertian(const vec3& a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}

	vec3 albedo;
};

//Defines properties of a metallic material
class metal :public material {
public:
	metal(const vec3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
		attenuation = albedo;
		return(dot(scattered.direction(), rec.normal) > 0);
	}
	vec3 albedo;
	float fuzz;
};

//Defines properties of a dielectric material (glasses)
class dielectric : public material {
public:
	float ref_idx;
	dielectric(const vec3& a, float ri) : albedo(a), ref_idx(ri) {}
	vec3 albedo;
	
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
		vec3 outward_normal;
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		float ni_over_nt;
		attenuation = vec3(1.0, 1.0, 1.0);
		vec3 refracted;
		float reflect_prob;
		float cosine;
		attenuation = albedo;

		// when ray shoot through object back into vacuum,
		// ni_over_nt = ref_idx, surface normal has to be inverted

		if (dot(r_in.direction(), rec.normal) > 0) {
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
            cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
            cosine = sqrt(1 - ref_idx*ref_idx*(1-cosine*cosine));
		}
		else {
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}

		//refracted ray exists
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
			reflect_prob = schlick(cosine, ref_idx);
		}
		//refracted ray does not exist
		else {
			//total internal reflection
			reflect_prob = 1.0;
		}

		//Using Fresnel's equations, can get reflections occuring due to total internal reflection

		if ((rand() / (RAND_MAX + 1.0)) < reflect_prob) {
			scattered = ray(rec.p, reflected);
		}
		else {
			scattered = ray(rec.p, refracted);
		}
		return true;
	}

};
