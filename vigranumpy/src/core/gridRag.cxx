/************************************************************************/
/*                                                                      */
/*                 Copyright 2011 by Ullrich Koethe                     */
/*                                                                      */
/*    This file is part of the VIGRA computer vision library.           */
/*    The VIGRA Website is                                              */
/*        http://hci.iwr.uni-heidelberg.de/vigra/                       */
/*    Please direct questions, bug reports, and contributions to        */
/*        ullrich.koethe@iwr.uni-heidelberg.de    or                    */
/*        vigra@informatik.uni-hamburg.de                               */
/*                                                                      */
/*    Permission is hereby granted, free of charge, to any person       */
/*    obtaining a copy of this software and associated documentation    */
/*    files (the "Software"), to deal in the Software without           */
/*    restriction, including without limitation the rights to use,      */
/*    copy, modify, merge, publish, distribute, sublicense, and/or      */
/*    sell copies of the Software, and to permit persons to whom the    */
/*    Software is furnished to do so, subject to the following          */
/*    conditions:                                                       */
/*                                                                      */
/*    The above copyright notice and this permission notice shall be    */
/*    included in all copies or substantial portions of the             */
/*    Software.                                                         */
/*                                                                      */
/*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND    */
/*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   */
/*    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          */
/*    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT       */
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,      */
/*    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR     */
/*    OTHER DEALINGS IN THE SOFTWARE.                                   */
/*                                                                      */
/************************************************************************/

#define PY_ARRAY_UNIQUE_SYMBOL vigranumpygraphs_PyArray_API
#define NO_IMPORT_ARRAY
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>


#include "export_graph_visitor.hxx"
#include "export_graph_rag_visitor.hxx"
#include "export_graph_algorithm_visitor.hxx"
#include "export_graph_shortest_path_visitor.hxx"
#include "export_graph_hierarchical_clustering_visitor.hxx"

#include "vigra/numpy_array.hxx"
#include "vigra/numpy_array_converters.hxx"
#include "vigra/adjacency_list_graph.hxx"
#include "vigra/graph_features.hxx"
#include "vigra/grid_rag_visualization.hxx"



namespace python = boost::python;

namespace vigra{





    template<unsigned int DIM>
    struct ExportFeatureExtractor{


        typedef GridRagFeatureExtractor<DIM, UInt32> GridRagFeatureExtractorType;

        static void exportFeatureExtractor(
            const std::string & clsName
        ){

            python::class_<GridRagFeatureExtractorType>(
                clsName.c_str(),
                python::no_init
            )

            .def("nAccumulatedFeatures",registerConverters(&nAccumulatedFeatures<float>))
            .def("accumulatedFeatures", registerConverters(&accumulatedFeatures<float>),
                (
                    python::arg("data"),
                    python::arg("out") = python::object()
                )
            )
            .def("nGeometricFeatures", &GridRagFeatureExtractorType::nGeometricFeatures)
            .def("geometricFeatures", registerConverters(&geometricFeatures),
                (
                    python::arg("out") = python::object()
                )
            )
            .def("nTopologicalFeatures", &GridRagFeatureExtractorType::nTopologicalFeatures)
            .def("topologicalFeatures", registerConverters(&topologicalFeatures),
                (
                    python::arg("out") = python::object()
                )
            )
            .def("nCyclePropergationFeatures", &GridRagFeatureExtractorType::nCyclePropergationFeatures)
            .def("cyclePropergationFeatures", registerConverters(&cyclePropergationFeatures),
                (
                    python::arg("edgeIndicators"),
                    python::arg("out") = python::object()
                )
            )
            //.def("nUcmTransformFeatures", &GridRagFeatureExtractorType::nUcmTransformFeatures)
            .def("ucmTransformFeatures", registerConverters(&ucmTransformFeatures),
                (
                    python::arg("edgeIndicators"),
                    python::arg("wardness"),
                    python::arg("out") = python::object()
                )
            )
            ;



            python::def("gridRagFeatureExtractor", registerConverters(&factory),
                python::return_value_policy<python::manage_new_object>()
            );
        }


        static NumpyAnyArray ucmTransformFeatures(
            const GridRagFeatureExtractorType & extractor,
            const NumpyArray<2, float>  & edgeIndicators,
            const NumpyArray<1, float>  & wardness,
            NumpyArray<2, float> out
        ){
            TinyVector<UInt32,2> outShape(extractor.edgeNum(),extractor.nUcmTransformFeatures(edgeIndicators,wardness));
            out.reshape(outShape);
            extractor.ucmTransformFeatures(edgeIndicators,wardness, out);
            return out;
        }



        static NumpyAnyArray cyclePropergationFeatures(
            const GridRagFeatureExtractorType & extractor,
            const NumpyArray<1, float>  & edgeIndicators,
            NumpyArray<2, float> out
        ){
            TinyVector<UInt32,2> outShape(extractor.edgeNum(),extractor.nCyclePropergationFeatures());
            out.reshape(outShape);
            extractor.cyclePropergationFeatures(edgeIndicators, out);
            return out;
        }


        // the factory
        static GridRagFeatureExtractorType * factory(
            const AdjacencyListGraph & graph,
            NumpyArray<DIM, UInt32> labels
        ){
            if(graph.edgeNum() != graph.maxEdgeId() + 1){
                throw std::runtime_error("graph.edgeNum() == graph.maxEdgeId() +1 is violated");
            }
            return new GridRagFeatureExtractorType(graph, labels);
        }


        template<class DATA_TYPE>
        static UInt64 nAccumulatedFeatures(
            const GridRagFeatureExtractorType & extractor,
            const NumpyArray<DIM, DATA_TYPE>  & data
        ){
            return extractor.nAccumulatedFeatures<DATA_TYPE>();
        }

        template<class DATA_TYPE>
        static python::tuple accumulatedFeatures(
            const GridRagFeatureExtractorType & extractor,
            const NumpyArray<DIM, DATA_TYPE>  & data,
            NumpyArray<2, float> out
        ){


            TinyVector<UInt32,2> outShape(extractor.edgeNum(),extractor.nAccumulatedFeatures<DATA_TYPE>());
            out.reshape(outShape);
            auto fnames = extractor.accumulatedFeatures(data, out);
            return python::make_tuple(out, fnames);
        }

        static python::tuple geometricFeatures(
            const GridRagFeatureExtractorType & extractor,
            NumpyArray<2, float> out
        ){
            TinyVector<UInt32,2> outShape(extractor.edgeNum(), extractor.nGeometricFeatures());
            out.reshape(outShape);
            std::vector<std::string> fnames = extractor.geometricFeatures(out);
            return python::make_tuple(out, fnames);
        }


        static python::tuple topologicalFeatures(
            const GridRagFeatureExtractorType & extractor,
            NumpyArray<2, float> out
        ){
            TinyVector<UInt32,2> outShape(extractor.edgeNum(), extractor.nTopologicalFeatures());
            out.reshape(outShape);
            auto fnames = extractor.topologicalFeatures(out);
            return python::make_tuple(out, fnames);
        }
    };




    template<unsigned int DIM>
    void defineGridRag(){

        std::string clsName;
        if (DIM == 2)
            clsName = "GridRagFeatureExtractor2D";
        if (DIM == 3)
            clsName = "GridRagFeatureExtractor3D";
        if (DIM == 4)
            clsName = "GridRagFeatureExtractor4D";

        ExportFeatureExtractor<DIM>::exportFeatureExtractor(clsName) ;
    }


    template void defineGridRag<2>();
    template void defineGridRag<3>();

    void pyFindSliceEdges(
        SliceEdges & sliceEdges,
        const NumpyArray<2, UInt32> labels
    ){
        sliceEdges.findSlicesEdges(labels);

    }

    NumpyAnyArray pyVisibleEdges(
        SliceEdges & sliceEdges,
        NumpyArray<1, Int32> out
    ){
        out.reshapeIfEmpty(TinyVector<MultiArrayIndex,1>(sliceEdges.nVisibleEdges()));
        sliceEdges.visibleEdges(out);
        return out;
    }

    UInt64 pyNLinesTotal(const SliceEdges & sliceEdges){
        return sliceEdges.nLines();
    }
    UInt64 pyNLines(const SliceEdges & sliceEdges, const UInt64 edgeId){
        return sliceEdges.nLines(edgeId);
    }

    NumpyAnyArray pyLineIds(
        const SliceEdges & sliceEdges, 
        const UInt64 edgeId,
        NumpyArray<1, UInt64> out
    ){
        out.reshapeIfEmpty(TinyVector<MultiArrayIndex,1>(sliceEdges.nLines(edgeId)));
        sliceEdges.lineIds(edgeId, out);
        return out;
    }

    NumpyAnyArray pyLine(
        const SliceEdges & sliceEdges, 
        const UInt64 lineId,
        NumpyArray<1, typename SliceEdges::Coord > out
    ){
        out.reshapeIfEmpty(TinyVector<MultiArrayIndex,1>(sliceEdges.lineSize(lineId)));
        sliceEdges.line(lineId, out);
        return out;
    }

    void defineVisualization(){




        python::class_< std::vector < std::string > >("StringVector")
        .def(python::vector_indexing_suite<std::vector< std::string > >());


        python::class_<SliceEdges>("SliceEdges",python::init<const AdjacencyListGraph  &>())
            .def("findSlicesEdges", registerConverters(&pyFindSliceEdges))
            .def("nVisibleEdges",&SliceEdges::nVisibleEdges)
            .def("visibleEdges",registerConverters(&pyVisibleEdges),
                (
                    python::arg("out") = python::object()
                )
            )
            .def("nLines",&pyNLinesTotal)
            .def("nLines",&pyNLines)
            .def("lineIds",registerConverters(&pyLineIds),
                (
                    python::arg("edgeId"),
                    python::arg("out") = python::object()
                )
            )
            .def("lineSize",&SliceEdges::lineSize)
            .def("line",registerConverters(&pyLine),
                (
                    python::arg("lineId"), 
                    python::arg("out") = python::object()
                )
            )
            ;

        ;
    }

} 



