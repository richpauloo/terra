// Copyright (c) 2018-2019  Robert J. Hijmans
//
// This file is part of the "spat" library.
//
// spat is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// spat is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with spat. If not, see <http://www.gnu.org/licenses/>.

#include <fstream>
#include <numeric>
#include "spatVector.h"

#ifdef useGDAL
#include "gdal_priv.h"
#endif

#ifdef useRcpp

#include <Rcpp.h>
// Rcpp::depends(RcppProgress)
#include "progress.hpp"
#include "progress_bar.hpp"
#endif


class SpatCategories {
	public:
		std::vector<double> levels;
		std::vector<std::string> labels;
};


class SpatColors {
	public:
		std::vector<unsigned> id;
		std::vector<std::string> value;
};

class RasterSource {
    private:
//		std::ofstream ofs;
	public:
#ifdef useGDAL
		GDALDataset* gdalconnection;
#endif
		bool open_read;
		bool open_write;

		RasterSource();

//		void fsopen(std::string filename);
//		bool fswrite(std::vector<double> &v);
//		void fsclose();

		unsigned ncol, nrow, nlyr, nlyrfile;
		SpatExtent extent;
		std::string crs;
		std::vector<unsigned> layers;
		std::vector<std::string> names;

		//std::vector< std::vector<double> values;
        std::vector<double> values;
        std::vector<int64_t> ivalues;
        std::vector<bool> bvalues;

		std::vector<bool> hasRange;
		std::vector<double> range_min;
		std::vector<double> range_max;
		std::vector<double> time;
		std::vector<bool> hasAttributes;
		std::vector<SpatDataFrame> atts;
		std::vector<bool> hasCategories;
		std::vector<SpatCategories> cats;
		std::vector<bool> hasColors;
		std::vector<SpatColors> cols;

		bool memory;
		bool hasValues;
		std::string filename;
		//unsigned nlyrfile;

		// for native files
		std::string datatype; // also for writing gdal
		std::string driver;
		std::string bandorder;
		std::string byteorder;

		double NAflag;

		std::vector<bool> has_scale_offset;
		std::vector<double> scale;
		std::vector<double> offset;

		std::vector<RasterSource> subset(std::vector<unsigned> lyrs);
		std::vector<double> getValues(unsigned lyr);
		void setRange();
		void resize(unsigned n);
};


class BlockSize {
	public:
		std::vector<unsigned> row;
		std::vector<unsigned> nrows;
		unsigned n;
};

class SpatRaster {

	protected:
		SpatExtent extent;
		SpatExtent window;
		std::string crs;

	public:

#ifdef useRcpp
		Progress* pbar;
#endif

////////////////////////////////////////////////////
// properties and property-like methods for entire object
////////////////////////////////////////////////////
		//unsigned nrow, ncol;
		std::vector<RasterSource> source;

		BlockSize bs;
		BlockSize getBlockSize(unsigned n, unsigned steps=0);

		SpatMessages msg;
		void setError(std::string s) { msg.setError(s); }
		void addWarning(std::string s) { msg.addWarning(s); }
		bool hasError() { return msg.has_error; }
		bool hasWarning() { return msg.has_warning; }

		//double NA = std::numeric_limits<double>::quiet_NaN();

		unsigned ncol();
		unsigned nrow();
		size_t size() { return ncol() * nrow() * nlyr() ; }
		SpatExtent getExtent() { return extent; }
		void setExtent(SpatExtent e) { extent = e ; }
		void setExtent(SpatExtent ext, bool keepRes=false, std::string snap="");  // also set it for sources?
		std::string getCRS() { return(crs); }
		void setCRS(std::string _crs);
		bool is_lonlat();
		bool could_be_lonlat();
		bool is_global_lonlat();

		std::vector<double> resolution();
		SpatRaster setResolution(double xres, double yres);
		double ncell() { return nrow() * ncol(); }
		double xres() { return (extent.xmax - extent.xmin) / ncol() ;}
		double yres() { return (extent.ymax - extent.ymin) / nrow() ;}
		std::vector<double> origin();
		unsigned nlyr();

		// only no values allowed with a single RasterSource
		bool hasValues() { return source[0].hasValues ; };
		std::vector<double> getValues();
		bool setValues(std::vector<double> _values);
		void setRange();
////////////////////////////////////////////////////
// property like methods for RasterSources
////////////////////////////////////////////////////
		std::vector<std::string> filenames();
		bool isSource(std::string filename);
		std::vector<bool> inMemory();


////////////////////////////////////////////////////
// property like methods for Layers
////////////////////////////////////////////////////

		std::vector<bool> hasRange();
		std::vector<double> range_min();
		std::vector<double> range_max();
		std::vector<std::string> getNames();
		bool setNames(std::vector<std::string> names);

////////////////////////////////////////////////////
// constructors
////////////////////////////////////////////////////

		SpatRaster();
		SpatRaster(unsigned _nrow, unsigned _ncol, unsigned _nlyr, SpatExtent ext, std::string _crs);
		SpatRaster(std::vector<unsigned> rcl, std::vector<double> ext, std::string _crs);

		SpatRaster(std::vector<std::string> fname);
		SpatRaster(std::string fname);
		SpatRaster(RasterSource s);
		void setSource(RasterSource s);
		void setSources(std::vector<RasterSource> s);
		//SpatRaster(const SpatRaster& x);

        SpatRaster deepCopy();
        SpatRaster geometry(long nlyrs=-1);

		bool constructFromFile(std::string fname);
		bool constructFromFiles(std::vector<std::string> fnames);
		bool constructFromFileGDAL(std::string fname);
		bool constructFromSubDataSets(std::string filename, std::vector<std::string> sds);

		void addSource(SpatRaster x);
		SpatRaster combineSources(SpatRaster x);
		SpatRaster subset(std::vector<unsigned> lyrs, SpatOptions &opt);

////////////////////////////////////////////////////
// helper methods
////////////////////////////////////////////////////

		void spatinit();

		bool compare_geom(SpatRaster x, bool lyrs, bool crs, bool warncrs=false, bool ext=true, bool rowcol=true, bool res=false);

		std::vector<double> cellFromXY (std::vector<double> x, std::vector<double> y);
		double cellFromXY(double x, double y);
		std::vector<double> cellFromRowCol(std::vector<unsigned> row, std::vector<unsigned> col);
		double cellFromRowCol(unsigned row, unsigned col);
		std::vector<double> cellFromRowColCombine(std::vector<unsigned> row, std::vector<unsigned> col);
		double cellFromRowColCombine(unsigned row, unsigned col);
		std::vector<double> yFromRow(std::vector<unsigned> &row);
		double yFromRow(unsigned row);
		std::vector<double> xFromCol(std::vector<unsigned> &col);
		double xFromCol(unsigned col);

		std::vector<unsigned> colFromX(std::vector<double> &x);
		unsigned colFromX(double x);
		std::vector<unsigned> rowFromY(std::vector<double> &y);
		unsigned rowFromY(double y);
		std::vector< std::vector<double>> xyFromCell( std::vector<double> &cell);
		std::vector< std::vector<double>> xyFromCell( double cell);
		std::vector< std::vector<unsigned>> rowColFromCell(std::vector<double> &cell);
        std::vector<unsigned> sourcesFromLyrs(std::vector<unsigned> lyrs);
		int sourceFromLyr(unsigned lyr);
		std::vector<unsigned> findLyr(unsigned lyr);

        std::vector<unsigned> nlyrBySource();
        std::vector<unsigned> lyrsBySource();
        unsigned nsrc();

		void createCategories(unsigned layer);
		std::vector<bool> hasCategories();
		void setCategories(unsigned layer, std::vector<std::string> labs, std::vector<double> levs);
		std::vector<SpatCategories> getCategories();
		SpatCategories getLayerCategories(unsigned layer);

		void createAttributes(unsigned layer);
		std::vector<bool> hasAttributes();
		void setAttributes(unsigned layer, SpatDataFrame df);
		std::vector<SpatDataFrame> getAttributes();
		SpatDataFrame getLayerAttributes(unsigned layer);

		double valuesCell(double);
		double valuesCell(int, int);
		std::vector<double> valuesCell(std::vector<double>);
		std::vector<double> valuesRow(int);

		bool isLonLat();
		bool couldBeLonLat();
		bool isGlobalLonLat();

////////////////////////////////////////////////////
// read and write
////////////////////////////////////////////////////

		// for all sources
		bool readStart();
		std::vector<double> readValues(unsigned row, unsigned nrows, unsigned col, unsigned ncols);
		std::vector<double> readBlock(BlockSize bs, unsigned i);
		bool readStop();

		bool writeStart(SpatOptions &opt);
		bool writeValues(std::vector<double> &vals, unsigned startrow, unsigned nrows, unsigned startcol, unsigned ncols);
		bool writeValues2(std::vector<std::vector<double>> &vals, unsigned startrow, unsigned nrows, unsigned startcol, unsigned ncols);
		bool writeStop();
		bool writeHDR(std::string filename);

		bool writeStartGDAL(std::string filename, std::string format, std::string datatype, bool overwrite, SpatOptions &opt);
		bool fillValuesGDAL(double fillvalue);
		bool writeValuesGDAL(std::vector<double> &vals, unsigned startrow, unsigned nrows, unsigned startcol, unsigned ncols);
		bool writeStopGDAL();

		bool writeStartBinary(std::string filename, std::string datatype, std::string bandorder, bool overwrite);
		bool writeValuesBinary(std::vector<double> &vals, unsigned startrow, unsigned nrows, unsigned startcol, unsigned ncols);

		bool writeValuesMem(std::vector<double> &vals, unsigned startrow, unsigned nrows, unsigned startcol, unsigned ncols);

		// binary (flat) source
		std::vector<double> readValuesBinary(unsigned src, unsigned row, unsigned nrows, unsigned col, unsigned ncols);
		std::vector<double> readSampleBinary(unsigned src, unsigned srows, unsigned scols);
		std::vector<std::vector<double>> readCellsBinary(unsigned src, std::vector<double> cells);

		// gdal source
		std::vector<double> readValuesGDAL(unsigned src, unsigned row, unsigned nrows, unsigned col, unsigned ncols);
		std::vector<double> readGDALsample(unsigned src, unsigned srows, unsigned scols);
		std::vector<std::vector<double>> readRowColGDAL(unsigned src, const std::vector<unsigned> &rows, const std::vector<unsigned> &cols);

		bool readStartGDAL(unsigned src);
		bool readStopGDAL(unsigned src);
		std::vector<double> readChunkGDAL(unsigned src, unsigned row, unsigned nrows, unsigned col, unsigned ncols);

		void openFS(std::string const &filename);

		SpatRaster writeRaster(SpatOptions &opt);
		SpatRaster writeRasterGDAL(std::string filename, std::string format, std::string datatype, bool overwrite, SpatOptions &opt);
		SpatRaster writeRasterBinary(std::string filename, std::string datatype, std::string bandorder, bool overwrite);

		bool canProcessInMemory(unsigned n);
		unsigned chunkSize(unsigned n);

		void fill(double x);

////////////////////////////////////////////////////
// main methods
////////////////////////////////////////////////////

        std::vector<std::vector<double>> adjacent(std::vector<double> cells, std::string directions, bool include);
 		SpatRaster aggregate(std::vector<unsigned> fact, std::string fun, bool narm, SpatOptions &opt);
		SpatExtent align(SpatExtent e, std::string snap);
		SpatRaster area(SpatOptions &opt);
		SpatRaster arith(SpatRaster x, std::string oper, SpatOptions &opt);
		SpatRaster arith(double x, std::string oper, bool reverse, SpatOptions &opt);
		SpatRaster arith(std::vector<double> x, std::string oper, bool reverse, SpatOptions &opt);
		SpatRaster apply(std::vector<unsigned> ind, std::string fun, bool narm, std::vector<std::string> nms, SpatOptions &opt);

		SpatVector as_polygons(bool values, bool narm);
		SpatVector as_points(bool values, bool narm);
		SpatRaster atan_2(SpatRaster x, SpatOptions &opt);

		SpatRaster buffer(double d, SpatOptions &opt);
		SpatRaster clamp(double low, double high, bool usevalue, SpatOptions &opt);
		SpatRaster cover(SpatRaster x, double value, SpatOptions &opt);
		SpatRaster crop(SpatExtent e, std::string snap, SpatOptions &opt);
		SpatRaster cum(std::string fun, bool narm, SpatOptions &opt);
        SpatRaster disaggregate(std::vector<unsigned> fact, SpatOptions &opt);
		SpatRaster distance(SpatOptions &opt);
		SpatRaster distance(SpatVector p, SpatOptions &opt);

		SpatRaster edges(bool classes, std::string type, unsigned directions, SpatOptions &opt);
		SpatRaster extend(SpatExtent e, SpatOptions &opt);
		std::vector<std::vector<std::vector<double>>> extractVector(SpatVector v, std::string method="");
		std::vector<std::vector<double>> extractCell(std::vector<double> &cell);
        std::vector<std::vector<double>> extractXY(std::vector<double> &x, std::vector<double> &y, std::string method);
		SpatRaster flip(bool vertical, SpatOptions &opt);
		SpatRaster focal(std::vector<double> w, double fillvalue, bool narm, std::string fun, SpatOptions &opt);
		std::vector<double> focal_values(std::vector<unsigned> w, double fillvalue, unsigned row, unsigned nrows);
		std::vector<std::vector<double>> freq(bool bylayer);
		bool get_aggregate_dims(std::vector<unsigned> &fact, std::string &message);
		std::vector<unsigned> get_aggregate_dims2(std::vector<unsigned> fact);
		std::vector<std::vector<double> > get_aggregates(std::vector<double> &in, size_t nr, std::vector<unsigned> dim);
//		std::vector<double> compute_aggregates(std::vector<double> &in, size_t nr, std::vector<unsigned> dim, std::function<double(std::vector<double>&, bool)> fun, bool narm);
		SpatDataFrame global(std::string fun, bool narm);
		SpatRaster gridDistance(SpatOptions &opt);
		SpatRaster gridCostDistance(SpatRaster cost, SpatOptions &opt);

		SpatRaster init(std::string value, bool plusone, SpatOptions &opt);
		SpatRaster init(double value, SpatOptions &opt);
		SpatRaster isnot(SpatOptions &opt);
        std::vector<double> line_cells(SpatGeom& g);
		SpatRaster logic(SpatRaster x, std::string oper, SpatOptions &opt);
		SpatRaster logic(bool x, std::string oper, SpatOptions &opt);
		SpatRaster mask(SpatRaster x, bool inverse, double maskvalue, double updatevalue, SpatOptions &opt);
		SpatRaster mask(SpatVector x, bool inverse, double maskvalue, double updatevalue, SpatOptions &opt);
		SpatRaster math(std::string fun, SpatOptions &opt);
		SpatRaster math2(std::string fun, unsigned digits, SpatOptions &opt);

		SpatRaster modal(std::vector<double> add, std::string ties, bool narm, SpatOptions &opt);

        std::vector<double> polygon_cells(SpatGeom& g);
		SpatRaster quantile(std::vector<double> probs, bool narm, SpatOptions &opt);

		SpatRaster range(std::vector<double> add, bool narm, SpatOptions &opt);
		SpatRaster rasterize(SpatVector p, double background, SpatOptions &opt);
		SpatRaster reclassify(std::vector<std::vector<double>> rcl, unsigned right, bool lowest, bool othersNA, SpatOptions &opt);
		SpatRaster reclassify(std::vector<double> rcl, unsigned nc, unsigned right, bool lowest, bool othersNA, SpatOptions &opt);
		std::vector<double> readSample(unsigned src, unsigned srows, unsigned scols);
		SpatRaster rotate(bool left, SpatOptions &opt);

		SpatRaster sampleRegular(unsigned size);
		SpatRaster collapse(SpatRaster x, SpatOptions &opt);

		SpatRaster shift(double x, double y, SpatOptions &opt);
		SpatRaster summary(std::string fun, bool narm, SpatOptions &opt);
		SpatRaster summary_numb(std::string fun, std::vector<double> add, bool narm, SpatOptions &opt);

		SpatRaster transpose(SpatOptions &opt);
		SpatRaster trig(std::string fun, SpatOptions &opt);
		SpatRaster trim(unsigned padding, SpatOptions &opt);
		std::vector<std::vector<double>> unique(bool bylayer);
		SpatRaster project(std::string newcrs, std::string method, SpatOptions &opt);
		SpatRaster warp(SpatRaster x, std::string method, SpatOptions &opt);
		SpatDataFrame zonal(SpatRaster x, std::string fun, bool narm);

};


class SpatRasterCollection {
	public:
		std::vector<SpatRaster> x;
		SpatRasterCollection() {};
		SpatRasterCollection(size_t n) { x.resize(n); };
		size_t size() { return x.size(); }
		void resize(size_t n) { x.resize(n); }
		void push_back(SpatRaster r) { x.push_back(r); };

		SpatRaster merge(SpatOptions &opt);
		SpatRaster moscaic(SpatOptions &opt);

};


class SpatRasterStack {
	public:
		std::vector<SpatRaster> x;
		SpatRasterStack() {};
		SpatRasterStack(size_t n) { x.resize(n); };
		size_t size() { return x.size(); }
		void resize(size_t n) { x.resize(n); }
		void push_back(SpatRaster r) { x.push_back(r); };
};



/*
SpatRaster SQRT() {
	SpatRaster r = *this;
	std::transform(r.values.begin(), r.values.end(), r.values.begin(), (double(*)(double)) sqrt);
	return r;
}

SpatRaster SQRTfree(SpatRaster* g) {
	SpatRaster r = *g;
	std::transform(r.values.begin(), r.values.end(), r.values.begin(), (double(*)(double)) sqrt);
	return r;
}
*/
