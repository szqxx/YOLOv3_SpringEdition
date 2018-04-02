﻿#include"yolo_v3.h"

int YoloDetect(image img, int* _net, float threshold, float* result, int result_sz) {
	network* net = (network*)_net;
	
	image im = img;
	image sized = letterbox_image(im, net->w, net->h);

	layer l = net->layers[net->n - 1];


	float *prediction = network_predict(*net, sized.data);


	int nboxes = 0;
	int letterbox = 0;
	float hier_thresh = 0.5;
	detection *dets = get_network_boxes(&net, im.w, im.h, threshold, hier_thresh, 0, 1, &nboxes, letterbox);
	do_nms_sort_v3(dets, nboxes, l.classes, 0.45F);
	int result_idx = 0;
	for (size_t i = 0; i < nboxes; ++i) {
		box b = dets[i].bbox;
		int const obj_id = max_index(dets[i].prob, l.classes);
		float const prob = dets[i].prob[obj_id];

		if (prob > threshold) {
			result[result_idx * 6 + 0] = obj_id;
			result[result_idx * 6 + 1] = prob;
			result[result_idx * 6 + 2] = MAX((double)0, (b.x - b.w / 2.)*im.w);
			result[result_idx * 6 + 3] = MAX((double)0, (b.y - b.h / 2.)*im.h);
			result[result_idx * 6 + 4] = b.w*im.w;
			result[result_idx * 6 + 5] = b.h*im.h;
			result_idx++;
		}
	}
	free_detections(dets, nboxes);
	if (sized.data)
		free(sized.data);
	

	return (int)result_idx;
}

/*
*	@YoloLoad
*	@param1 : cfg 파일 경로
*	@param2: weights 파일 경로
*	@comment : C++의 경우는 int*로 network파일을 담아두면 됩니다.
*				, C#의 경우 IntPtr의 자료형으로 받으면 됩니다.
*	@return : network class pointer
*/

DLL_MACRO int* YoloLoad(char* cfgfile, char* weightsfile) {
	network* net = NULL;//(network*)malloc(sizeof(network));
	//memset(net, 0, sizeof(network));
	*net = parse_network_cfg(cfgfile);
	load_weights(net, weightsfile);
	set_batch_network(net, 1);
	srand(920217);
	return (int*)net;
}
/*
*	@YoloDetect
*	@param1 : image 파일 경로
*	@param2: network
*	@param3: threshold of confidence value
*	@param4: float array
*	@param5 : size of array
*	@comment : result is looks like [class,confidence,x,y,w,h][class,confidence,x,y,w,h]...
*	@return : number of detect
====get rect example
int sz=YoloDetect(path, network, threshold, result,6*100);
-> it picks 100 boxes ordered by confidence value
for (int i = 0; i < sz; i ++) {
int kind = result[i * 6 + 0];
int cval = result[i * 6 + 1];
int left = result[i * 6 + 2];
int top = result[i * 6 + 3];
int width = result[i * 6 + 4];
int height = result[i * 6 + 5];
}
====
*/

DLL_MACRO int YoloDetectFromFile(char* img_path, int* _net, float threshold, float* result, int result_sz) {
	image im = load_image_color(img_path, 0, 0);
	int r=YoloDetect(im, _net, threshold, result, result_sz);
	free_image(im);
	return r;
}

DLL_MACRO int YoloDetectFromImage(float* data,int w,int h,int c, int* _net, float threshold, float* result, int result_sz) {
	image im;
	im.data=data;
	im.w=w;
	im.h=h;
	im.c=c;
	return YoloDetect(im, _net, threshold, result, result_sz);
}
//__declspec(dllexport) float YoloLoss(char* cfg, char* weights, char* image_list_file) {
//	network* pnet= (network*)YoloLoad(cfg, weights);
//	network net = *pnet;
//	list *vlist = get_paths(image_list_file);
//	char **vpaths = (char**)list_to_array(vlist);
//	data vbuffer;
//	layer l = net.layers[net.n - 1];
//	int classes = l.classes;
//	float jitter = l.jitter;
//	load_args vargs = { 0 };
//	vargs.w = net.w;
//	vargs.h = net.h;
//	vargs.paths = vpaths;
//	vargs.n = vlist->size;
//	vargs.m = vlist->size;
//	vargs.classes = classes;
//	vargs.jitter = jitter;
//	vargs.num_boxes = l.max_boxes;
//	vargs.d = &vbuffer;
//	vargs.type = DETECTION_DATA;
//	vargs.threads = 8;
//
//	vargs.angle = net.angle;
//	vargs.exposure = net.exposure;
//	vargs.saturation = net.saturation;
//	vargs.hue = net.hue;
//	pthread_t vload_thread = load_data(vargs);
//	pthread_join(vload_thread, 0);
//	float vloss = train_network_no_backward(net, vbuffer);
//
//	free_data(vbuffer);
//	free_network(net);
//	free(pnet);
//	return vloss;
//}
//__declspec(dllexport) void YoloVisualization(char* img_path, int* _net) {
//	image img = load_image_color(img_path, 0, 0);
//	network* net = (network*)_net;
//	image sized = resize_image(img, net->w, net->h);
//
//	network_visualization(*net, sized.data);
//}