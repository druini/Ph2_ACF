#include <array>

class Histogram {

    // Histogram

    std::string name;
    std::string description;
    std::array<std::string> axisNames;
};


template <class Container>
void drawHist(
    const Container& data, 
    std::string name, 
    std::string title, 
    double minValue, 
    double maxValue,
    const std::string& xLabel = "", 
    const std::string& yLabel = "", 
) {
    TH1F* h = new TH1F(name.c_str(), desc.c_str(), data.size(), minValue, maxValue);
    h->SetXTitle(xLabel);
    h->SetYTitle(yLabel);
    for (size_t i = 0; i < data.size(); ++i) 
        h->SetBinContent(i, *(data.data() + i));
    Canvas* c = new Canvas((name + "_canvas").c_str(), desc.c_str(), 600, 600);
    h->Draw("HIST");
    c->Write();
}

template <class T>
void drawHist2D(
    std::xtensor<T, 2> data, 
    const std::string& name, 
    const std::string& title, 
    double xMin,
    double xMax,
    double yMin, 
    double yMax,
    const std::string& xLabel = "", 
    const std::string& yLabel = "", 
    const std::string& zLabel = "",
    bool reverseYAxis = false
) {
    TH2F* h = new TH2F(name.c_str(), title.c_str(), RD53B<Flavor>::nCols, 0, RD53B<Flavor>::nCols, RD53B<Flavor>::nRows, 0, RD53B<Flavor>::nRows);
    h->SetXTitle(xLabel);
    h->SetYTitle(yLabel);
    j->SetZTitle(zLabel);
    for (size_t i = 0; i < data.shape()[0]; ++i)
        for (size_t j = 0; j < data.shape()[1]; ++j)
            h->Fill(i, j, data(i, j));
    Canvas* c = new Canvas((name + "_canvas").c_str(), title.c_str(), 600, 600);
    h->Draw("COLZ");
    if (reverseYAxis) {
        h->GetYaxis()->SetLabelOffset(999);
        h->GetYaxis()->SetTickLength(0);
        // Redraw the new axis
        gPad->Update();
        TGaxis *newaxis = new TGaxis(gPad->GetUxmin(),
                                        gPad->GetUymax(),
                                        gPad->GetUxmin()-0.001,
                                        gPad->GetUymin(),
                                        h->GetYaxis()->GetXmin(),
                                        h->GetYaxis()->GetXmax(),
                                        510,"+");
        newaxis->SetLabelOffset(-0.03);
        newaxis->Draw();
    }
    c->Write();
}

template <class T>
void drawPixelHist(std::xtensor<T, 2> data, const std::string& name, const std::string& title, const std::string& zLabel = "") {
    drawHist2D(data, name, title, 0, RD53B<Flavor>::nCols, 0, RD53B<Flavor>::nRows, "Column", "Row", zLabel, true);
}


template <class Container>
void drawHist2D(const Container& data, std::string name, std::string title, double minValue, double maxValue) {
    TH1F* h = new TH1F(name.c_str(), desc.c_str(), data.size(), minValue, maxValue);
    for (size_t i = 0; i < data.size(); ++i) 
        h->SetBinContent(i, *(data.data() + i));
    Canvas* c = new Canvas((name + "_canvas").c_str(), desc.c_str(), 600, 600);
    h->Draw("HIST");
    c->Write();
}

void draw

template <class HistType>
class Histogram : HistogramBase {
    static const std::string _prefix = "canvas_"

public:
    explicit Histogram(HistType* hist) 
      : _histogram(hist) 
      , _canvas(hist->getName(), "Canvas", 600, 600) 
    {}
    
    template <class... Args>
    explicit Histogram(Args&&... args) : _histogram(new HistType{std::forward<Args>(args)...}) {}

    HistType* hist() { return _histogram.get(); }

    Canvas* canvas() const { return _canvas.get(); }

    void setAxisLabels(const char* xTitle, const char* yTitle = "", const char* zTitle = "") {
        histogram->SetXTitle(xTitle);
        histogram->SetYTitle(yTitle);
        histogram->SetZTitle(zTitle);
    }

    template <class Container>
    void fill(const Container& data) {
        for (size_t i = 0; i < data.size(); ++i)
            histogram->SetBinContent(i + 1, *(data->data() + i));
    }

    void draw(const char* drawOptions = "", bool reverseY = false) {
        _canvas->cd();
        histogram->Draw(drawOptions);
        if (reverseY)
            reverseYAxis();
        _canvas->Write();
    }

private:
    void reverseYAxis()
    {
        // Remove the current axis
        _histogram->GetYaxis()->SetLabelOffset(999);
        _histogram->GetYaxis()->SetTickLength(0);
        // Redraw the new axis
        gPad->Update();
        TGaxis *newaxis = new TGaxis(gPad->GetUxmin(),
                                        gPad->GetUymax(),
                                        gPad->GetUxmin()-0.001,
                                        gPad->GetUymin(),
                                        _histogram->GetYaxis()->GetXmin(),
                                        _histogram->GetYaxis()->GetXmax(),
                                        510,"+");
        newaxis->SetLabelOffset(-0.03);
        newaxis->Draw();
    }

private:
    std::unique_ptr<HistType> _histogram;
    std::unique_ptr<Canvas> _canvas = nullptr;
};
