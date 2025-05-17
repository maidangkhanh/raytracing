#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include <thread>
#include <mutex>
#include <atomic>

// Atomic counter to track progress
std::atomic<int> scanlines_completed(0);

class camera
{
public:
	double aspect_ratio = 1.0; // Ratio of image width over height
	int image_width = 100; //  Rendered image width in pixel count
	int samples_per_pixel = 10; // Count of random samples for each pixel
	int    max_depth = 10;   // Maximum number of ray bounces into scene

	double vfov = 90; // Vertical view angle (field of view)
	point3 lookfrom = point3(0, 0, 0);   // Point camera is looking from
	point3 lookat = point3(0, 0, -1);  // Point camera is looking at
	vec3   vup = vec3(0, 1, 0);     // Camera-relative "up" direction

	double defocus_angle = 0; // Variation angle of rays through each pixel
	double focus_dist = 10; // Distance from camera lookfrom point to plane of perfect focus

	void render(const hittable& world)
	{
		initialize();

		std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";


		for (int j = 0; j < image_height; j++)
		{
			std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
			for (int i = 0; i < image_width; i++)
			{
				color pixel_color(0, 0, 0);
				for (int sample = 0; sample < samples_per_pixel; sample++)
				{
					ray r = get_ray(i, j);
					pixel_color += ray_color(r, max_depth, world);
				}
				write_color(std::cout, pixel_samples_scale * pixel_color);
			}
		}
		std::clog << "\rDone.                 \n";
	}

	// Main rendering function
	void render_image_threaded(const hittable& world) {
		initialize();

		std::mutex stream_mutex;
		{
			std::lock_guard<std::mutex> lock(stream_mutex);
			std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";
		}
;
		
		// Calculate height for each thread section
		int section_height = image_height / 5;

		// Create threads
		std::vector<std::thread> threads;

		for (int t = 0; t < 5; t++) 
		{
			int start_height = t * section_height;
			int end_height = (t == 4) ? image_height : (t + 1) * section_height; // Handle remaining rows in last thread

			threads.push_back(std::thread(&camera::render_section, this, start_height, end_height, std::ref(world), std::ref(stream_mutex)));
		}

		// Wait for all threads to complete
		for (auto& thread : threads) 
		{
			thread.join();
		}

		// Now write all pixels in the correct order
		{
			std::lock_guard<std::mutex> lock(stream_mutex);
			for (int j = 0; j < image_height; j++) {
				for (int i = 0; i < image_width; i++) {
					write_color(std::cout, pixel_buffer[j][i]);
				}
			}
			std::clog << "\rDone.                 \n";
		}
	}

private:
	int		image_height;	// Render image height
	double pixel_samples_scale; // Color scale factor for a sum of pixel samples
	point3	center;			// Camera center
	point3	pixel00_loc;	//Location of pixel 0, 0
	vec3	pixel_delta_u;	// Offset to pixel to the right
	vec3	pixel_delta_v;	// Offset to pixel below
	vec3   u, v, w;              // Camera frame basis vectors

	vec3 defocus_disk_u;	// Defocus disk horizontal radius
	vec3 defocus_disk_v;		// Defocus disk vertical radius

	std::vector<std::vector<color>> pixel_buffer;
	std::mutex buffer_mutex;
	// Mutex for synchronizing console output
	std::mutex console_mutex;


	void initialize()
	{
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;
		pixel_samples_scale = 1.0 / samples_per_pixel;
		center = lookfrom;
		pixel_buffer.resize(image_height, std::vector<color>(image_width));

		// Determine viewport dimensions.
		//auto focal_length = (lookfrom - lookat).length();
		auto theta = degrees_to_radians(vfov);
		auto h = std::tan(theta / 2);
		auto viewport_height = 2 * h * focus_dist;
		auto viewport_width = viewport_height * (double(image_width) / image_height);

		// Calculate the u,v,w unit basis vectors for the camera coordinate frame.
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);
	
		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		auto viewport_u = viewport_width * u;
		auto viewport_v = viewport_height * -v;

		// Calculate the horizontal and verical delta vectors from pixel to pixel.
		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;

		// Calculate the location of the upper left pixel.
		auto viewport_upper_left =
			center - (focus_dist *w) - viewport_u / 2 - viewport_v / 2;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
		
		// Calculate the camera defocus disk basis vectors.
		auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
		defocus_disk_u = u * defocus_radius;
		defocus_disk_v = v * defocus_radius;
	}

	// Construct a camera ray originating from the origin and directed at randomly sampled
	// point around the pixel location i, j.
	ray get_ray(int i, int j) const
	{
		auto offset = sample_square();
		auto pixel_sample = pixel00_loc
			+ ((i + offset.x()) * pixel_delta_u)
			+ ((j + offset.y()) * pixel_delta_v);

		auto ray_origin = (defocus_angle <=0)? center: defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		auto ray_time = rand_double();

		
		return ray(ray_origin, ray_direction, ray_time);
	}

	// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
	vec3 sample_square() const
	{
		return vec3(rand_double() - 0.5, rand_double() - 0.5, 0);
	}

	// Returns a random point in the camera defocus disk.
	point3 defocus_disk_sample() const 
	{
		auto p = random_in_unit_disk();
		return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
	}

	color ray_color(const ray& r,int depth, const hittable& world) const
	{
		// If we've exceeded the ray bounce limit, no more light is gathered.
		if (depth <= 0)
			return color(0, 0, 0);

		hit_record rec;
		if (world.hit(r, interval(0.001, infinity), rec))
		{
			ray scattered;
			color attenuation;
			if (rec.mat->scatter(r, rec, attenuation, scattered))
				return attenuation * ray_color(scattered, depth - 1, world);
			return color(0, 0, 0);
		}

		vec3 unit_direction = unit_vector(r.direction());
		auto a = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
	}

	void render_section(int start_height, int end_height, const hittable& world, std::mutex& stream_mutex) 
	{
		for (int j = start_height; j < end_height; j++) {
			for (int i = 0; i < image_width; i++) {
				color pixel_color(0, 0, 0);
				for (int sample = 0; sample < samples_per_pixel; sample++)
				{
					ray r = get_ray(i, j);
					pixel_color += ray_color(r, max_depth, world);
				}

				// Store in buffer instead of writing immediately
				{
					std::lock_guard<std::mutex> lock(buffer_mutex);
					pixel_buffer[j][i] = pixel_samples_scale * pixel_color;
				}
			}

			int completed = ++scanlines_completed;
			{
				std::lock_guard<std::mutex> lock(console_mutex);
				std::clog << "\rScanlines remaining: " << image_height - completed << ' ' << std::flush;
			}
		}
	}
};

#endif // !CAMERA_H
