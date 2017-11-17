#include "TrackerCtrl.h"

String FormatSize(int64 bytes, bool kibi = false) 
{
	const char* base1000[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "YB", "ZB" };
	const char* base1024[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "YiB", "ZiB" }; 
	
	int base = !kibi ? 1000 : 1024;
	int exponent = floor(log2(bytes) / log2(base));
	
	return bytes < base
			? FormatInt64(bytes)  << " " << (!kibi ? base1000[0] : base1024[0]) 
			: Format("%.1f %s", bytes / pow(base, exponent), !kibi ? base1000[exponent] : base1024[exponent]);
}

void TrackerCtrl::Track(const Item& t)
{
	auto& ti = GetAdd(t);
	auto& tt = ti.item;

	if(!tt.InProgress()) {
		if(tt.IsSuccess()) {
			tt.message = t_("Finished");
			ti.end = GetSysTime();
			ti.elapsed.Set(ti.end - ti.start);
		}
		else
		if(tt.IsFailure()) {
			tt.message = Format(t_("Failed (%s)"), tt.message);
			ti.end = GetSysTime();
			ti.elapsed.Set(ti.end - ti.start);
		}
		else
		if(tt.IsStarted()) {
			tt.message = t_("Starting...");
			ti.start = GetSysTime();
			ti.elapsed = Null;
		}
		else		
		if(tt.IsRestarted()) {
			tt.message = t_("Restarting...");
			ti.start = GetSysTime();
			ti.elapsed = Null;
		}
		Filter();
	}
	else
	if(tt.InProgress()) {
		tt.message = tt.type == Item::PUT 
			? t_("Uploading") 
			: t_("Downloading");
		ti.elapsed.Set(GetSysTime() - ti.start);
		auto cursor = itemlist.Find(tt.id);
		if(cursor != -1) {
			auto* pr = (ProgressIndicator*) itemlist.GetCtrl(cursor, PROGRESS);
			pr->Set(tt.done, tt.size);
			itemlist.SetArray(cursor, ti());
			Summary();
		}
	}
}

void TrackerCtrl::Set(const Item& t, int i, const Value& v)
{
	auto cursor = itemlist.Find(t.id);
	if(cursor >= 0)
		itemlist.Set(cursor, i, v);
}

TrackerCtrl::TI& TrackerCtrl::GetAdd(const Item& t)
{
	auto i  = items.Find(t.id);
	auto id = t.id;
	if(i >= 0) {
		items.SetKey(i, id);
		auto& ti = items.Get(t.id);
		ti.item = pick(t);
		return ti;
	}
	else {
		for(int i = 0; i < items.GetCount(); i++) {
			auto& ti = items[i];
			auto& tt = ti.item;
			if(tt.source == t.source && tt.destination == t.destination) {
				tt = pick(t);
				items.SetKey(i, id);
				return ti;
			}
		}
		TI ti;
		ti.item = pick(t);
		return items.Add(id, ti); 
	}	
}

void TrackerCtrl::Filter()
{
 	SetFocus();
	if(itemlist.GetCount()) 
		itemlist.Clear();
	auto ii = filter.GetIndex();
	for(auto& ti : items) {
		auto& t = ti.item;
		switch(ii) {
			case FINISHED:   
				if(t.IsSuccess()) 
					Sync(ti);	
				break;
			case FAILED:
				if(t.IsFailure()) 
					Sync(ti);
				break;
			case ACTIVE:
				if(t.IsStarted() ||  t.IsRestarted() || t.InProgress()) 
					Sync(ti);
				break;
			case ALL:
			default:
				Sync(ti);
				break;
		}
	}
	Summary();
}

void TrackerCtrl::Sync(const TI& ti)
{
	itemlist.AddArray(ti());
	auto& t = ti.item;
	auto cursor = itemlist.Find(t.id);
	auto* pr = (ProgressIndicator*) itemlist.GetCtrl(cursor, PROGRESS);
	if(pr)
		switch(t.status){
			case Item::FAILURE:
				pr->SetColor(LtRed()).Set(1,1);
				pr->Percent(false);
				break;
			case Item::SUCCESS:
				pr->SetColor(Green()).Set(1,1);
				break;
			case Item::RESTARTED:
			case Item::PROGRESS:
				pr->Set(t.done, t.size);
				break;
			default:
				break;
		}
}

void TrackerCtrl::Summary()
{
	int fl = 0, sc = 0, pr = 0;
	tsize = 0;
	tdone = 0;
	for(auto& ti : items) {
		tsize += ti.item.size;
		tdone += ti.item.done;
		switch(ti.item.status) {
			case Item::FAILURE:
				fl++;
				break;
			case Item::SUCCESS:
				sc++;
				break;
			case Item::STARTED:
			case Item::RESTARTED:
			case Item::PROGRESS:
				pr++;
				break;
			default:
				break;
		}
		
	}
	status = Format(t_(" Total Transfers: %d (Active: %d, Finished: %d, Failed: %d) Done: %s, Total: %s"),
				items.GetCount(),
				pr,
				sc,
				fl,
				FormatSize(tdone), 
				FormatSize(tsize));
}

void TrackerCtrl::Populate(int i, One<Ctrl>& ctrl)
{
	auto& prg = ctrl.Create<ProgressIndicator>();
	prg.Percent();
	prg.Set(0, INT_MAX);
}

void TrackerCtrl::MainToolBar(Bar& bar)
{
	if(stdbar) {
		bar.Add(t_("Retry"), CtrlImg::Toggle(), [=]{ WhenRetry(); });
		bar.Add(t_("Cancel"), CtrlImg::Remove(), [=]{ WhenCancel(); });
		bar.GapRight();
		bar.Add(filter.SizePos(), 120);
	}
}
void TrackerCtrl::ContextMenu(Bar& bar)
{
	if(stdmenu) {
		bar.Sub(t_("Columns"), THISFN(ColumnMenu));
		bar.Sub(t_("Filter"),  THISFN(FilterMenu));
		if(IsCursor()) {
			bar.Separator();
			ActionMenu(bar);
		}
		if(WhenBar) bar.Separator();
	}
	WhenBar(bar);
}

void TrackerCtrl::ColumnMenu(Bar& bar)
{
	HeaderCtrl& header = itemlist.HeaderObject();
	for(int i = 0; i < header.GetCount(); i++){
		bool visible = header.IsTabVisible(i);
		auto label = header[i].GetText();
		bar.Add(label, [=] { 
			itemlist.HeaderObject().ShowTab(i, !visible); }).Check(visible); 
		};
}

void TrackerCtrl::FilterMenu(Bar& bar)
{
	for(auto i = 0; i < filter.GetCount(); i ++) {
		auto key = filter.GetKey(i);
		auto val = filter.GetValue(i);
		bar.Add(val.To<String>(), [=]{ filter.SetIndex(key); Filter(); })
			.Check(filter.GetIndex() == key);
	}
}

void TrackerCtrl::ActionMenu(Bar& bar)
{
	auto& t = Get();
	bar.Add(t.IsFailure(),  t_("Retry"), [=]{ WhenRetry(); })
		.Image(CtrlImg::Toggle());
	bar.Add(t.InProgress(), t_("Cancel"), [=]{ WhenCancel(); })
		.Image(CtrlImg::Remove()); 
}

void TrackerCtrl::Serialize(Stream& s)
{
	Ctrl::Serialize(s);
	itemlist.SerializeSettings(s);
}

TrackerCtrl::TrackerCtrl()
{
	CtrlLayout(*this);
	itemlist.AddColumn(t_("ID"));
	itemlist.AddColumn(t_("Source"));
	itemlist.AddColumn(t_("Destination"));
	itemlist.AddColumn(t_("Progress")).Ctrls(THISFN(Populate));
	itemlist.AddColumn(t_("Transferred"));
	itemlist.AddColumn(t_("Status"));
	itemlist.AddColumn(t_("Start Time"));
	itemlist.AddColumn(t_("Elapsed Time"));
	itemlist.AddColumn(t_("Finish Time"));
	itemlist.ColumnWidths("5 40 40 20 20 20 20 20 20");
	itemlist.WhenBar = Proxy(WhenBar);
	itemlist.EvenRowColor();
	itemlist.SetLineCy(GetStdFont().GetHeight() + 8);
	itemlist.WhenBar = THISFN(ContextMenu);
	filter.Add(ACTIVE,  t_("Active"));
	filter.Add(FINISHED,t_("Finished"));
	filter.Add(FAILED,  t_("Failed"));
	filter.Add(ALL,     t_("All"));
	filter.WhenAction = THISFN(Filter);
	filter.GoEnd();	
	AddFrame(toolbar);
	toolbar.Set(THISFN(MainToolBar));
	status.NoSizeGrip();
	stdmenu = false;
	stdbar  = true;
	tsize   = 0;
	tdone   = 0;
	Summary();
}

ValueArray TrackerCtrl::Item::operator()() const
{
	ValueArray row = { id, source, destination, Null, Null, message };
	return pick(row);
}

ValueArray TrackerCtrl::TI::operator()() const
{
	auto FTime = [=](const Time& t) { return Format("%02d:%02d:%02d", t.hour, t.minute, t.second); };
	ValueArray row = { 
		item.id, 
		item.source, 
		item.destination, 
		Null, 
		Format(t_("%s of %s"), FormatSize(item.done), FormatSize(item.size)), 
		item.message, 
		FTime(start),
		FTime(elapsed),		
		FTime(end)
	};
	return pick(row);
}

TrackerCtrl::Item ConvertResult(Ftp::Result r, bool put)
{
	TrackerCtrl::Item t;
	t.id = r.GetId();
	t.source = r.GetName();
	t.destination = AppendFileName(GetHomeDirectory(), GetFileName(r.GetName()));
	t.size = r.GetTotal();
	t.done = r.GetDone();
	t.type = put ? TrackerCtrl::Item::PUT : TrackerCtrl::Item::GET;
	t.message = r.GetErrorDesc();
	t.error = r.GetError();
	t.data  = Null;
	if(r.IsStarted())
		t.status = TrackerCtrl::Item::STARTED;
	else
	if(r.InProgress())
		t.status = TrackerCtrl::Item::PROGRESS;
	else
	if(r.IsRestarted())
		t.status = TrackerCtrl::Item::RESTARTED;
	else
	if(r.IsSuccess())
		t.status = TrackerCtrl::Item::SUCCESS;
	else
	if(r.IsFailure())
		t.status = TrackerCtrl::Item::FAILURE;
	return pick(t);
}	
