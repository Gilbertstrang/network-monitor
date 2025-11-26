#include <network-monitor/transport-network.h>


namespace NetworkMonitor {

TransportNetwork::TransportNetwork() = default;

TransportNetwork::~TransportNetwork() = default;

bool Station::operator==(const Station& other) const {
    return id == other.id;
}

bool Line::operator==(const Line& other) const {
    return id == other.id;
}

bool Route::operator==(const Route& other) const {
    return id == other.id;
}

TransportNetwork::TransportNetwork(const TransportNetwork& copied) = default;
TransportNetwork::TransportNetwork(const TransportNetwork& moved) = default;

TransportNetwork& TransportNetwork::operator=(const TransportNetwork& copied) = default;
TransportNetwork& TransportNetwork::operator=(TransportNetwork&& moved) = default;


bool TransportNetwork::AddStation(const Station& station) {
    if (stations_.count(station.id) > 0) return false;

    GraphNode node;
    node.stationId = station.id;
    node.name = station.name;
    node.passengerCount = 0;

    stations_[station.id] = node;
    return true;
}

bool TransportNetwork::AddLine(const Line& line) {
    if (lines_.count(line.id)) return false;

    for (size_t i = 0; i < line.routes.size(); ++i) {
        if (routes_.count(line.routes[i].id) > 0) return false;

        for (const Id& stationid : line.routes[i].stops) {
            if (stations_.count(stationid) == 0) return false;

        }

        for (size_t j = 0; j < line.routes[i].stops.size() - 1; ++j) {
            Id curStation = line.routes[i].stops[j];
            Id nextStation = line.routes[i].stops[j+1];

            GraphEdge edge;
            edge.lineId = line.routes[i].lineId;
            edge.routeId = line.routes[i].id;
            edge.nextStationId = nextStation;

            stations_[curStation].edges.push_back(edge);
        }

        routes_[line.routes[i].id] = line.routes[i];
    }
    
    lines_[line.id] = line;
    
    return true;
}

bool TransportNetwork::RecordPassengerEvent(const PassengerEvent& event) {
    if (stations_.count(event.stationId) == 0) return false;

    switch(event.type) {
        case PassengerEvent::Type::In:
            stations_[event.stationId].passengerCount++;
            break;
        case PassengerEvent::Type::Out:
            stations_[event.stationId].passengerCount--;
            break;
    }
    return true;
}

long long int TransportNetwork::GetPassengerCount(const Id& station) const {
    if (stations_.count(station) == 0) throw std::runtime_error("station not found");

    return stations_.at(station).passengerCount;

    
}

std::vector<Id> TransportNetwork::GetRoutesServingStation(const Id& station) const {
    std::vector<Id> result {};
    
    if (stations_.count(station) == 0) return result;

    for (const auto& route : routes_) {
        auto it = std::find(route.second.stops.begin(), route.second.stops.end(), station); //.second because it is a pair of id,route
        if (it != route.second.stops.end()) {
            result.push_back(station);
        }
    }

    return result;

}   

bool TransportNetwork::SetTravelTime(
    const Id& stationA,
    const Id& stationB,
    const unsigned int travelTime
) {
    if (stations_.count(stationA) == 0) return false;
    if (stations_.count(stationB) == 0) return false;

    if (TransportNetwork::AreAdjacent(stationA, stationB)) {
        std::string key = TransportNetwork::MakeEdgeKey(stationA, stationB);
        travelTimes_[key] = travelTime;
    }
}

unsigned int TransportNetwork::GetTravelTime(const Id& stationA, const Id& stationB) const {
    if (stationA == stationB) return 0;
    std:: string key = TransportNetwork::MakeEdgeKey(stationA, stationB);
    
    auto it = travelTimes_.find(key);
    if (it != travelTimes_.end()) {
        return travelTimes_.at(key);
    }
    return 0;
}

unsigned int TransportNetwork::GetTravelTime(
    const Id& line,
    const Id& route,
    const Id& stationA,
    const Id& stationB
) const {
    if (lines_.count(line) == 0) return 0;
    if (routes_.count(route) == 0) return 0;
    if (stationA == stationB) return 0;

    //todo
    
}


}