#pragma once

#include "stupid/common.h"
#include "stupid/renderer/render_types.h"
#include "stupid/math/linear.h"

#include <vulkan/vulkan.h>

typedef struct StWindow StWindow;

/// Buffer allocated by VulkanMemoryAllocator.
typedef struct StRendererVulkanBuffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	usize size;
	VkDeviceAddress address;
} StRendererVulkanBuffer;

/// This basically manages the usage of shaders.
/// @todo Make this less horrible.
typedef struct STUPID_A32 StRendererVulkanPipeline {
	VkPipeline handle;
	VkPipelineLayout layout;

	struct {
		char **pShaderData;
		VkDynamicState *pDynamicStates;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;
		VkPipelineRenderingCreateInfo rendering_info;
		VkGraphicsPipelineCreateInfo graphics_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment_state;
		VkPipelineColorBlendStateCreateInfo color_blend_state_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineTessellationStateCreateInfo tessellation_info;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineVertexInputStateCreateInfo vertex_input_info;
		VkPipelineLayoutCreateInfo layout_info;
	} info;
} StRendererVulkanPipeline;

/// Configuration for stRendererVulkanImageCreate
/// @see StRendererVulkanImage
typedef struct StRendererVulkanImageOptions {
	/// The type of this vulkan image.
	/// @note This should probably be VK_IMAGE_TYPE_2D.
	VkImageType type;

	/// The format for this vulkan image.
	/// @note For color images, this should probably be VK_FORMAT_R8G8B8STUPID_A8_UNORM.
	/// @note For depth images, this is determined by the GPU.
	VkFormat format;

	/// What this vulkan image will be used for.
	/// @note For color images, this should probably be VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT.
	/// @note For depth images, this should probably be VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.
	VkImageUsageFlags usage_flags;

	/// Where this image will be stored in memory.
	VkMemoryPropertyFlags memory_flags;

	/// This seems redundant, but this is what type of stuff this vulkan image will be used for.
	/// @note For color images, this should probably be VK_IMAGE_ASPECT_COLOR_BIT.
	/// @note For color images, this should probably be VK_IMAGE_ASPECT_DEPTH_BIT.
	VkImageAspectFlags aspect_flags;

	/// Width of this image.
	u32 width;

	/// Height of this image.
	u32 height;

	/// Whether this image requires a vulkan image view or not.
	bool view;
} StRendererVulkanImageOptions;

/// @brief vulkan images are used for a lot of things, but in most cases they can be thought of as buffers.
/// Color images are just buffers that can be drawn to the screen, and depth images are used to check how far things are from the camera.
/// Image views are usually required to correctly interpret data from a vulkan image.
/// @note A vulkan image view is not required if you are just using it as a buffer.
/// @note A vulkan image is another name for a vulkan attachment.
typedef struct STUPID_A8 StRendererVulkanImage {
	/// @brief Internal vulkan image handle.
	/// Color images can be thought of as pictures (or buffers), and depth images calculate depth in 3D rendering.
	/// @note You cant use this without a vulkan image view.
	VkImage handle;

	/// Required in order to use most vulkan image types.
	VkImageView view;

	/// Driver side information about vulkan memory management.
	/// @todo Implement more centralized memory management (something like StRendererVulkanMemory in StRendererVulkanContext).
	VkDeviceMemory memory;

	/// The memory layout of this vulkan image.
	/// @note VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL for viewable images.
	/// @note VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL for transfering memory from this image to an image or buffer.
	/// @note VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL for transfering memory from an image or buffer to this image.
	/// @note VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for showing the image on the screen.
	VkImageLayout layout;

	/// Properties of this vulkan image.
	/// @note This is mostly just used to recreate an image.
	StRendererVulkanImageOptions options;
} StRendererVulkanImage;

/// This manages the vulkan images actually shown to the screen.
typedef struct StRendererVulkanSwapchain {
	/// vulkan swapchain handle.
	VkSwapchainKHR handle;

	/// Swapchain image format.
	/// @note This should probably be VK_FORMAT_B8G8R8STUPID_A8_UNORM;
	VkSurfaceFormatKHR image_format;

	/// Used to draw to a window.
	VkSurfaceKHR surface;

	/// These vulkan images are the images that are actually shown on the screen.
	/// @note Its probably just a waste of memory to have more than 4.
	StRendererVulkanImage pImages[4];

	/// Used to determine whats closer to the camera.
	StRendererVulkanImage depth_attachment;

	VkPresentModeKHR previous_present_mode;
	VkPresentModeKHR present_mode;

	/// whether the swapchain is currently being recreated
	STUPID_ATOMIC bool is_recreating;

	/// number of vulkan images to buffer when drawing (i.e. 2 would be double buffering)
	u32 image_count;

	/// Current width of the swapchain.
	u32 swapchain_width;

	/// Current height of the swapchain.
	u32 swapchain_height;

	/// maximum frames to be queued at once
	u8 max_frames_in_flight;

	/// the index of the current frame being rendered
	u8 current_frame;
} StRendererVulkanSwapchain;

/// Information about swapchain capabilities, and available features.
/// @see StRendererVulkanDevice, StRendererVulkanSwapchain
typedef struct StRendererVulkanSwapchainSupport {
	/// Usable vulkan surface formats.
	/// @note A VkFormatSurfaceKHR object contains a single VkFormat, and a single compatible VkColorSpaceKHR.
	/// @see format_count
	VkSurfaceFormatKHR pFormats[303];

	/// Available present modes.
	/// @note VK_PRESENT_MODE_IMMEDIATE_KHR uncaps the framerate.
	/// @note VK_PRESENT_MODE_MAILBOX_KHR buffers frames (usually the best format).
	/// @note VK_PRESENT_MODE_FIFO_KHR is vsync (always available).
	/// @note VK_PRESENT_MODE_FIFO_RELAXED_KHR is relaxed vsync.
	/// @note VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR should not be used.
	/// @note VK_PRESENT_MODE_CONTINUOUS_REFRESH_KHR should not be used.
	/// @see present_mode_count
	VkPresentModeKHR pPresentModes[6];

	/// GPU surface capabilities.
	VkSurfaceCapabilitiesKHR capabilities;

	/// Number of available image formats.
	/// @note Max is 303.
	/// @see pFormats
	u32 format_count;

	/// Number of available present modes.
	/// @note Max is 6.
	/// @see pPresentModes
	u32 present_mode_count;
} StRendererVulkanSwapchainSupport;

/// Physical GPU.
typedef struct StRendererVulkanDevice {
	/// Physical GPU handle.
	/// @see logical_device
	VkPhysicalDevice physical_device;

	/// Driver side GPU handle.
	/// @see physical_device
	VkDevice logical_device;

	/// Available swapchain features and capabilities.
	StRendererVulkanSwapchainSupport swapchain_support;

	/// Queue from the GPU in charge of graphics operations.
	/// @note Look it up.
	/// @see present_queue
	/// @see transfer_queue
	/// @see compute_queue
	VkQueue graphics_queue;

	/// Queue from the GPU in charge of present operations.
	/// @note Look it up.
	/// @see graphics_queue
	/// @see transfer_queue
	/// @see compute_queue
	VkQueue present_queue;

	/// Queue from the GPU in charge of memory transfer operations.
	/// @note Look it up.
	/// @see graphics_queue
	/// @see present_queue
	/// @see compute_queue
	VkQueue transfer_queue;

	/// Queue from the GPU in charge of compute operations.
	/// @note Look it up.
	/// @note This queue is usually unnecessary.
	/// @see graphics_queue
	/// @see present_queue
	/// @see transfer_queue
	VkQueue compute_queue;

	/// Graphics queue index.
	/// @note Look it up.
	/// @see present_queue_index
	/// @see transfer_queue_index
	/// @see compute_queue_index
	u32 graphics_queue_index;

	/// Present queue index.
	/// @note Look it up.
	/// @see graphics_queue_index
	/// @see transfer_queue_index
	/// @see compute_queue_index
	u32 present_queue_index;

	/// Transfer queue index.
	/// @note Look it up.
	/// @see graphics_queue_index
	/// @see present_queue_index
	/// @see compute_queue_index
	u32 transfer_queue_index;

	/// Compute queue index.
	/// @note Look it up.
	/// @see graphics_queue_index
	/// @see present_queue_index
	/// @see transfer_queue_index
	u32 compute_queue_index;

	/// @brief Command buffer collection.
	/// This can record rendering operations for execution.
	/// @note Multithreading requires one of these for each thread.
	/// @see StRendererVulkanCommandBuffer
	/// @todo Make this multithreading compatible.
	VkCommandPool graphics_command_pool;

	/// GPU properties.
	VkPhysicalDeviceProperties properties;

	/// GPU memory properties.
	VkPhysicalDeviceMemoryProperties memory;

	/// Available GPU features.
	VkPhysicalDeviceFeatures features;

	/// Format to use for depth images.
	/// @note VK_FORMAT_D16_UNORM is used for reading, and can be used when VK_FORMAT_D32_SFLOAT isnt available.
	/// @note VK_FORMAT_X8_D24_UNORM_PACK32 shouldnt be used.
	/// @note VK_FORMAT_D32_SFLOAT is used for reading and should usually be used.
	/// @note VK_FORMAT_D16_UNORM_S8_UINT is used for writing and shouldnt be used.
	/// @note VK_FORMAT_D24_UNORM_S8_UINT is used for writing and shouldnt be used.
	/// @note VK_FORMAT_D32_SFLOAT_S8_UINT is used for writing and shouldnt be used.
	VkFormat depth_format;
} StRendererVulkanDevice;

/// Requirements for a physical GPU.
typedef struct StRendererVulkanDeviceRequirements {
	/// Required vulkan queues.
	/// @note Look it up.
        struct {
		/// capabilities of this queue family
		bool graphics, present, transfer, compute;
        } queue;

        u32 extension_count;
        const char **extensions;

        u32 layer_count;
        const char **layers;

        /// Required swapchain capabilities.
        StRendererVulkanSwapchainSupport swapchain_capabilities;
} StRendererVulkanDeviceRequirements;

/// An instance of the vulkan API.
/// @note There can only be one instance.
typedef struct StRendererVulkanBackend {
	/// vulkan instance handle.
	/// @note Destroy this last.
	VkInstance instance;

	/// vulkan custom allocation callbacks.
	/// @note If this is NULL (and it usually is), then vulkan will use the default allocation callbacks.
	VkAllocationCallbacks *pAllocator;

	/// Physical GPU.
	StRendererVulkanDevice device;

	/// Required vulkan layers.
	const char **required_layers;

	/// Required vulkan extensions.
	const char **required_extensions;

	/// Required vulkan device extensions.
	const char **required_device_extensions;

#ifdef _DEBUG
	/// vulkan validation layer logger.
	/// @note Debug only.
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
} StRendererVulkanBackend;

/// vulkan command buffer states.
typedef enum st_renderer_vulkan_command_buffer_state {
	/// the command buffer is ready to rumble
	ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_READY,

	/// the command buffer is recording commands
	ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING,

	/// the command buffer is receiving commands
	ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_IN_RENDERPASS,

	/// the command buffer has submitted the commands
	ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_SUBMITTED,

	/// the command buffer is not allocated
	ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED
} st_renderer_vulkan_command_buffer_state;

/**
 * @struct StRendererVulkanCommandBuffer
 * this basically stores all the vulkan rendering commands
 *
 * use StRendererVulkanCommandBufferBegin() to start recording rendering commands
 * use StRendererVulkanCommandBufferEnd() to finish recording rendering commands
 * use vkQueueSubmit() to execute rendering commands
 */
typedef struct StRendererVulkanCommandBuffer {
	/// Basically holds a list of commands for vulkan to execute.
	VkCommandBuffer handle;

	/// Current state of the command buffer.
	st_renderer_vulkan_command_buffer_state state;
} StRendererVulkanCommandBuffer;

/// A vulkan renderer instance.
typedef struct STUPID_A32 StRendererVulkanContext {
	/// An active vulkan backend.
	/// @note There can only be one of these.
	StRendererVulkanBackend *pBackend;

	/// Color images.
	StRendererVulkanImage *pColorAttachments;

	/// Depth images.
	StRendererVulkanImage *pDepthAttachments;

	/// vulkan images attached to the renderpass.
	/// @note The first one is always the current swapchain image.
	VkRenderingAttachmentInfo *pRenderingAttachments;

	/// Used to send commands to the GPU.
	/// @note There should be swapchain.image_count of these.
	StRendererVulkanCommandBuffer *pGraphicsCommandBuffers;

	/// vulkan graphics command buffer for the current frame.
	/// @note Equivalent to pGraphicsCommandBuffers[current_frame].
	StRendererVulkanCommandBuffer *pCurrentGraphicsCommandBuffer;

	/// @brief These are basically vulkan fences for GPU to GPU operations.
	/// These are used specifically to prevent asynchronous use of a swapchain image.
	/// @note There should be swapchain.image_count of these.
	VkSemaphore *pImageAvailableSemaphores;

	/// @brief These are basically vulkan fences for GPU to GPU operations.
	/// These are used specifically to prevent presentation of incomplete swapchain images.
	/// @note There should be swapchain.image_count of these.
	VkSemaphore *pQueueCompleteSemaphores;

	/// These are basically cameras.
	/// @note The main one is the first one.
	VkViewport *pViewports;

	/// Rendering regions.
	/// @note The main one is the first one.
	VkRect2D *pScissors;

	/// Rendering pipeline.
	/// @note Used to run vertex and fragment shaders.
	StRendererVulkanPipeline graphics_pipeline;

	/// Compute pipeline.
	/// @note Used to run compute shaders.
	StRendererVulkanPipeline compute_pipeline;

	/// @brief These are used to wait for a CPU resource to be released.
	/// These are used specifically for CPU to GPU operations.
	/// @note There should be swapchain.image_count of these.
	VkFence *pInFlightFences;

	/// The window associated with this vulkan context.
	StWindow *pWindow;

	/// Manages the images shown on the screen.
	StRendererVulkanSwapchain swapchain;

	/// Configuration for the dynamic renderpass.
	VkRenderingInfo rendering_info;

	/// Depth attachment associated with the dynamic renderpass.
	VkRenderingAttachmentInfo depth_attachment;

	/// GPU optimal depth format.
	VkFormat depth_format;

	/// Default background clear color.
	VkClearValue clear_value;

	/// @brief The index of the current vulkan swapchain image.
	/// This iterates from 0 to swapchain.image_count in an unpredictable order.
	/// @see current_frame
	u32 image_index;

	/// This is like image_index, but iterates normally.
	/// @see image_index
	u32 current_frame;

	/// whether the swapchain is currently being recreated or not
	bool recreating_swapchain;

	/// Internal variables shared among all renderer backends.
	StRendererValues rvals;
} StRendererVulkanContext;
