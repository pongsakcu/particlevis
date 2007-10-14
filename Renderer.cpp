#include "Renderer.h"

Renderer::Renderer(void) : coloring(NULL)
{
}

Renderer::~Renderer(void)
{
}

void Renderer::SetColorClass(RGBClassifier *classifier)
{
	coloring = classifier;
}
