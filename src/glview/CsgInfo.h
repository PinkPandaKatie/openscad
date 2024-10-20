#pragma once

#include "CSGNode.h"
#include "Tree.h"
#include "GeometryEvaluator.h"
#include "CSGTreeEvaluator.h"
#include "CSGTreeNormalizer.h"
#include "RenderSettings.h"
#include "printutils.h"

/*
   Small helper class for compiling and normalizing node trees into CSG products
 */
class CsgInfo
{
public:
  CsgInfo() = default;
  shared_ptr<class CSGProducts> root_products;
  shared_ptr<CSGProducts> highlights_products;
  shared_ptr<CSGProducts> background_products;

  bool compile_products(const Tree& tree) {
    auto& root_node = tree.root();
    GeometryEvaluator geomevaluator(tree);
    CSGTreeEvaluator evaluator(tree, &geomevaluator);
    shared_ptr<CSGNode> csgRoot = evaluator.buildCSGTree(*root_node);
    std::vector<shared_ptr<CSGNode>> highlightNodes = evaluator.getHighlightNodes();
    std::vector<shared_ptr<CSGNode>> backgroundNodes = evaluator.getBackgroundNodes();

    LOG("Compiling design (CSG Products normalization)...");
    CSGTreeNormalizer normalizer(RenderSettings::inst()->openCSGTermLimit);
    if (csgRoot) {
      shared_ptr<CSGNode> normalizedRoot = normalizer.normalize(csgRoot);
      if (normalizedRoot) {
        this->root_products.reset(new CSGProducts());
        this->root_products->import(normalizedRoot);
        LOG("Normalized CSG tree has %1$d elements", int(this->root_products->size()));
      } else {
        this->root_products.reset();
        LOG(message_group::Warning, "CSG normalization resulted in an empty tree");
      }
    }

    if (highlightNodes.size() > 0) {
      LOG("Compiling highlights (%1$i CSG Trees)...", highlightNodes.size());
      this->highlights_products.reset(new CSGProducts());
      for (auto& highlightNode : highlightNodes) {
        highlightNode = normalizer.normalize(highlightNode);
        this->highlights_products->import(highlightNode);
      }
    }

    if (backgroundNodes.size() > 0) {
      LOG("Compiling background (%1$i CSG Trees)...", backgroundNodes.size());
      this->background_products.reset(new CSGProducts());
      for (auto& backgroundNode : backgroundNodes) {
        backgroundNode = normalizer.normalize(backgroundNode);
        this->background_products->import(backgroundNode);
      }
    }
    return true;
  }
};
