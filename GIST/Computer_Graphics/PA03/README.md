# Overview

I implemented a ray-tracer!

# Implementations

Lambertian:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-38-37.png)

Here is the lambertian implementation with noise.

Lambertian (diffuse) is a material which just reflects a ray and does not emit any light.
Lambertian material reflects the incoming ray in a random direction.
Thus, to create such a ray, there's a need of the random class and some math.

Simply put, you create a random vector on the unit sphere (i.e. a sphere of radius 1),
and then add it to the normal vector of the hit point to avoid a reflection that would penetrate the material.

Indeed, when creating the random vector there is a 50% chance that he would be facing the wrong direction (i.e. he would go inside the lambertian sphere),
thus you add it to the normal of the hit point and it is sure to face the outside of the material.

Metal:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-39-02.png)

Here is the metal implementation with noise.

Metal is a material that is kind of like a mirror, it has a perfect reflection.
However, metal can have a certain roughness that would make it less mirrory.

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-39-19.png)
![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-39-35.png)

Here we have a mirror like metal (roughness = 0) and a metal with 1 of roughness.
What really changes with the roughness factor is the fuzzy reflection, it adds a bit of randomness in the reflection of the metal.
First create the secondary ray, you need to simply do a perfect reflection 
cf:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-39-56.png)

Once this ray is computed, we can add the randomness by creating a random vector and interpolate the two of them with the roughness factor.
Again, since this new ray could possibly go into the metal, we want to check the dot product between it and the normal and possibly invert the vector.

Dielectric:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-40-10.png)

This is the dielectric material (i.e. a glass ball).
The particularity of the glass material is that it can either reflect or refract the light. The reflection of the light is the same one as the one used in the metal material. 
For the refraction, the first thing you need to know is if the ray comes from the inside of the outside of the sphere, to be able to put your normal in the right direction (i.e. inverse it if the ray comes from inside). 
Afterwards, the goal is to compute the refracted ray by adding the "perpendicular" and the "parallel" ray.
We can pretty easily (with dot product and the index of refraction) create a perpendicular ray and then use it to derive the parallel one.
Finally you can add them up and you have your refracted ray.
Then, to decide if you refract or reflect a ray, you just use a bit of randomness and there are some angles in where it is not possible to refract so you just reflect.

Light:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-40-24.png)

Same, light with noise.
Since light emits light, the scattered rays become negligeable, that's why we can simply return false in the scatter function.

It does not attenuate the albedo and gives back a white light (i.e. glm::vec3(1.0f)).

Antialiasing:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-40-49.png)
![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-41-03.png)

Here are two images with some tweakings to them with 1024 samples by pixel with a Dielectric material not working well, here is the final version.

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-41-13.png)

For the antialiasing, instead of generating one ray per pixel, we will try to sample more of them.
However, we do not want to shoot them all in the same place (that would just work for the lambertian since the scattered ray is random).
Thus I apply a sort of offset to the sample, using a random that goes from
\- size of pixel / 2 to
\+ size of pixel / 2,
to change its direction a tiny bit.
In the end, I simply divide the final color by the number of samples to have an average.

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-41-23.png)

Indirect lighting:

I implemented it at the same time as the materials.
What happens is that in the integrate function, instead of checking the ray intersection one time, we will do it an x number of times by scaterring it each time.
Once we touch an object, we will affect the albedo, add the light emitted by the material and then scatter the ray and launch the operation again.
I have 4 stop cases for the while:
 1. No object are touched thus no ray to be scattered.
 2. The ray touches a light, since the DiffuseLight material emits light, the scattered ray is negligeable.
 3. The ray bounces more than a certain amount (I currently use 25) so that, if we are in a room full of mirrors, the computation is not infinite.
 4. The albedo value is too low (albedo < 0.1), meaning that even if the ray can still bounce, the color change would be negligeable (like that we rarely go to the 25 bounces). However this is useful since if we are kind of trapped in a glass ball, the albedo does not change so I want to keep going until I get out of the sphere.

Direct Sampling:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-41-34.png)
![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-41-48.png)


The goal of the direct sampling is to always verify if the touched material is under the light or in a shadow.
To do that, we can follow this principle:

![image](https://github.com/Rorolol-creator/Projects/blob/main/GIST/.images/Screenshot_2026-09-24_16-41-56.png)

For the implementation, we loop through all of the lights, generate the shadow ray (a ray going from the hit point to the light)
and add the emitted value of the point reached (i.e. if there is no obstructor, the point is on the light, thus emitted = 1, if not emitted = 0).
At the end of our loop we simply divide the vector (more like value at this point) by the number of lights to get an average of its "enlightenness".
Finally we add a line in our integrate function to add the direct sampling light to the color of the pixel.


![20240602_082526_IMG_4772](https://github.com/user-attachments/assets/75dbb203-3c0a-4ba3-85ae-a984217fcb11)

![image](https://github.com/user-attachments/assets/88604dce-4bae-4de8-8478-b5dc629eedaf)

![image](https://github.com/user-attachments/assets/55f4187b-9b10-47b6-b428-ca3306810835)

![image](https://github.com/user-attachments/assets/987d0ce9-5887-4bc7-a36b-019576a58432)

![image](https://github.com/user-attachments/assets/65a48d1f-8844-4f56-adb8-30209f78cd9e)

![20240602_060129_camphoto_684387517](https://github.com/user-attachments/assets/47aaa7e7-24be-40b7-9f33-0f01bd8765ea)

![image](https://github.com/user-attachments/assets/e374cff3-4382-4aac-b77d-bcb33673b850)

![20240602_060016_camphoto_1804928587](https://github.com/user-attachments/assets/5f030041-0d26-4ee1-9f8c-74508a85b581)
